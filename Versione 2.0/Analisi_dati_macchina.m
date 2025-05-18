clear all
clc
close all

%% Caricamento misure da file esterno
% nel file importati viene stampato anche il tempo di ogni campion, per la
% scelta corretta del tempo di campionamento. 
filename= 'pavimento.txt';                 % Nome del file dati con le misure
Measures = importdata(filename);            % Importa i dati come cell array di stringhe

% visto che viene stampato anche il tempo di ogni campione, con le righe
% sequenti selezionamo il valore della stringa relativo all'accelerazione,
% e poi facciamo il cast in double.
% Estrazione valori asse z (assumiamo che l'ultimo token di ogni riga sia il valore di accelerazione sull’asse z)
z_values = zeros(length(Measures),1);      % Preallocazione vettore z_values
for k = 1:length(Measures)
    line = Measures{k};                     % Prendo la riga k-esima
    tokens = strsplit(line, ' ');           % Divido la riga in token separati da spazio
    z_values(k) = str2double(tokens{end});  % Converto l’ultimo token in numero e lo salvo in z_values
end

% Parametri di campionamento
% intervallo di campionamento scelto in base ai valori stampati nei file
Ts = 0.002;  % Intervallo di campionamento (tempo tra due misure) in secondi (qui 500 Hz)
Fs = 1/Ts;   % Frequenza di campionamento (Hz)

%% FILTRAGIO DEL SEGNALE
% Prepara segnale (assumiamo che il valore sia in g, quindi moltiplichiamo per 9.81 per avere m/s^2)
z = z_values * 9.81;
L = length(z);               % Numero di campioni
t = (0:L-1)*Ts;             % Vettore tempi corrispondenti

% Rimuovo offset DC (la media), per togliere la componente gravitazionale
z = z - mean(z);            
% Applico una smoothing (media mobile su 10 campioni) per ridurre rumore ad alta frequenza
z = smoothdata(z, 'movmean', 10);  
figure(1)
plot(t, z);
grid on
title("Accelerazione z nel tempo (offset rimosso)");
xlabel("Tempo (s)");
ylabel("Accelerazione z (m/s^2)");

% Filtraggio passa basso Butterworth di 4° ordine per rimuovere rumori ad alta frequenza
Fc = 40; % Frequenza di taglio in Hz (più bassa rispetto prima per pulire meglio il segnale)
[b,a] = butter(4, Fc/(Fs/2));    % Calcolo i coefficienti del filtro Butterworth digitale
z_filt = filtfilt(b, a, z);      % Filtraggio bidirezionale per evitare ritardi di fase

figure(2)
plot(t, z, 'b', t, z_filt, 'r', 'LineWidth', 1.5)
legend('Segnale originale', 'Segnale filtrato')
title('Accelerazione filtrata nel tempo')
xlabel('Tempo (s)')
ylabel('Accelerazione (m/s^2)')
grid on

%% Calcolo accelerazione dinamica (già filtrata)
a_moto = z_filt;  % Ora il segnale è pulito e senza offset, rappresenta l'accelerazione dinamica

%% Integrazione numerica per ottenere velocità e posizione
v = cumtrapz(t, a_moto);   % Velocità tramite integrazione numerica della accelerazione
v = detrend(v);            % Rimuove eventuali trend lineari dovuti a errori cumulativi nell'integrazione
x = cumtrapz(t, v);        % Posizione tramite integrazione numerica della velocità
x = detrend(x);            % Rimuove eventuali trend lineari da posizione

%% Massa del sistema (necessaria per calcolo forze)
m = 1.74;   % Massa in kg

%% Preparazione dati per regressione
% Modello dinamico: m*a + c*v + k*x = 0
% Lo riorganizzo per la regressione: c*v + k*x = -m*a
Y = -m * a_moto;           % Vettore "uscita" (termine noto)
X = [v, x];                % Matrice regressori (velocità e posizione)

%% Regressione con vincolo di non negatività (per evitare valori fisicamente non plausibili)
params = lsqnonneg(X, Y);  % Risolve il problema di minimo quadrati con c,k >= 0

c = params(1);             % Coefficiente di smorzamento stimato (Ns/m)
k = params(2);             % Coefficiente di rigidezza stimato (N/m)

fprintf('Risultati regressione con vincolo c,k >= 0:\n');
fprintf('Smorzamento c = %.4f Ns/m\n', c);
fprintf('Rigidezza k = %.4f N/m\n', k);

%% Plot per confrontare dati e modello
a_model = c * v + k * x;   % Forza predetta dal modello smorzamento + rigidezza
a_model1 = (c/m) * v + (k/m) * x; 
figure(3)
plot(t, a_moto, 'b', t, a_model1, 'r--', 'LineWidth', 1.5)
legend('-m*a_{moto} (Y)', 'c*v + k*x (modello)')
title('Confronto tra dati e modello')
xlabel('Tempo (s)')
ylabel('Forza (N)')
grid on
