clear all
clc
close all

%% Caricamento misure da file esterno
filename= 'pavimento.txt';
Measures = importdata(filename);

% Estrazione valori asse z (assumendo che l'ultimo token di ogni riga sia z)
z_values = zeros(length(Measures),1);
for k = 1:length(Measures)
    line = Measures{k};
    tokens = strsplit(line, ' ');
    z_values(k) = str2double(tokens{end});
end

% Parametri di campionamento
Ts = 0.002;  % tempo di campionamento (500 Hz)
Fs = 1/Ts;

% Prepara segnale
z = z_values*9.81;
L = length(z);
t = (0:L-1)*Ts;

% Rimuovo offset DC
z = z - mean(z);
z = smoothdata(z);
figure(3)
plot(t,z);
grid on
title("acceleraizone z nel tempo");
xlabel("time");
ylabel("accelerazione z");


% Filtraggio passa basso Butterworth 4° ordine con frequenza di taglio Fc
Fc = 80; % Frequenza di taglio (Hz), modificabile
[b,a] = butter(4, Fc/(Fs/2));
z_filt = filtfilt(b, a, z);


figure(1)
plot(t, z, 'b', t, z_filt, 'r', 'LineWidth', 1.5)
legend('Segnale originale', 'Segnale filtrato')
title('Accelerazione nel tempo')
xlabel('Tempo (s)')
ylabel('Accelerazione (m/s^2)')
grid on

%% Analisi frequenziale sul segnale filtrato
Z_f = fft(z_filt);
P2 = abs(Z_f/L);
P1 = P2(1:floor(L/2)+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:floor(L/2))/L;

figure(2)
plot(f, P1, 'LineWidth', 2)
title('Spettro di ampiezza del segnale filtrato')
xlabel('Frequenza (Hz)')
ylabel('|P1(f)|')
grid on
xlim([0 150]) % adatta range frequenze

% Stima frequenza naturale come picco massimo nello spettro (escludendo DC)
[~, idx_peak] = max(P1(2:end));
fn = f(idx_peak+1);
fprintf('Frequenza naturale stimata: %.2f Hz\n', fn);

%% Stima smorzamento tramite logaritmic decrement
min_peak_dist = round(Fs/fn*0.8); % 80% del periodo stimato
[pks, locs] = findpeaks(z_filt, 'MinPeakDistance', min_peak_dist);

if length(pks) < 3
    warning('Non ci sono abbastanza picchi per stimare lo smorzamento');
    zeta = NaN;
else
    deltas = log(pks(1:end-1)./pks(2:end));
    delta_avg = mean(deltas);
    zeta = delta_avg / sqrt(4*pi^2 + delta_avg^2);
    if zeta < 0
        warning('Smorzamento stimato negativo, dati rumorosi o non smorzati.');
        zeta = NaN;
    else
        fprintf('Smorzamento stimato (zeta): %.4f\n', zeta);
    end
end

%% Integrazione numerica per velocità e posizione
v = cumtrapz(t, z_filt);
x = cumtrapz(t, v);

%% Regressione per stimare c/m e k/m
X = [v, x];
Y = -z_filt; % attenzione al segno per la formula dinamica

params = X \ Y;
a = params(1); % c/m
b = params(2); % k/m
% params = lsqnonneg(X, -z_filt);
% a = params(1);
% b = params(2);
%% Inserisci massa reale (modifica se la conosci)
m = 1.74;

c = a * m;
k = b * m;

fprintf('Risultati regressione:\n');
fprintf('Smorzamento c = %.4f Ns/m\n', c);
fprintf('Rigidezza k = %.4f N/m\n', k);
