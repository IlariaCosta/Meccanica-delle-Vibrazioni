clear all
clc
close all
%% Caricamento misure da file esterno
filename='datiprova.txt';
%filename='DATI-Cemento.txt';
%filename='DATI-Asfalto.txt';
%filename='DATI-Parquet.txt';


Measures = importdata(filename)*9.81; %% portiamo in m/s^2 

% Inizializzazione variabili
z = Measures(:,1);
z = z - mean(z);                        % rimuove offset DC

len = length(z);

Ts = 0.01; % tempo di campionamento Arduino
Fs = 1/Ts;
t = [0:Ts:(len-1)*Ts]'; % vettore dei tempi
%%
figure(1)
% t = t_dense;
Ts_d = Ts/10; % Periodo di campionamento
Fs_d = 1/Ts_d;
len_d = length(z);
%z = z_dense;

% Analisi frequenziale Fourier
any(isnan(z))
any(isinf(z))
Z_f = fft(z); % analisi frequenze
P2 = abs(Z_f/len_d);
P1 = P2(1:len_d/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs*(0:(len_d/2))/len_d;
plot(f,P1) 
title('Analisi frequenziale')
xlabel('f (Hz)')
ylabel('|P1(f)|')
xlim([0 50]) % per centrare la vista su un intervallo delle frequenze

%% 
% Creazione figura per l'istogramma
figure(2);
plot(z); % Impostiamo il numero di bin a 30 (puoi modificare il numero di bin)
title('Istogramma dei dati di accelerazione');
xlabel('Accelerazione (m/s^2)');
ylabel('Frequenza');
grid on;  % Aggiunge la griglia al grafico

%%
%% Integrazione numerica per ricavare velocità e posizione
v = cumtrapz(t, z);    % velocità integrando accelerazione
x = cumtrapz(t, v);    % posizione integrando velocità
 
%% Preparazione regressione
% Modello: z (accelerazione) = - (c/m)*v - (k/m)*x
 
X = [v, x];  % regressori: velocità e posizione
Y = z;       % uscita: accelerazione
 
params = X \ (-Y);  % regressione lineare (pseudo-inversa)
a = params(1);  % a = c/m
b = params(2);  % b = k/m
 
%% Stima di c, k (serve una stima della massa m)
m = 1; % <--- Inserisci qui la massa reale in kg (es. 1.5)
c = a * m;
k = b * m;
 
%% Stampa dei risultati
fprintf('Massa stimata: m = %.2f kg\n', m);
fprintf('Smorzamento:   c = %.4f Ns/m\n', c);
fprintf('Rigidezza:     k = %.4f N/m\n', k);