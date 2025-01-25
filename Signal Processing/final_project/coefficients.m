clear all;
% Continuous-time filter design
N = 3;
Oc = 0.7867;
poles = zeros(1, N); % Initialize pole array for clarity

% Calculate poles for a Butterworth filter
for k = 0:N-1
    poles(k+1) = Oc*exp(1i*(pi+2*k*pi)/(2*N))*exp(1i*(pi/2));
end

B_ct = Oc^3; % Continuous-time numerator coefficients (gain factor)
A_ct = real(poly(poles)); % Calculate polynomial from roots and ensure real coefficients

% Print the continuous-time filter coefficients
disp('Continuous-time B coefficients:');
disp(B_ct);
disp('Continuous-time A coefficients:');
disp(A_ct);

% Construct the continuous-time transfer function
Hc_ct = tf(B_ct, A_ct);

% Discretization using bilinear transform with a sampling frequency of 1 Hz
fs = 1; % Sampling frequency (Hz)
Hd_dt = c2d(Hc_ct, 1/fs, 'tustin'); % Discretize using the bilinear transform

% Print the digital filter coefficients
[num_d, den_d] = tfdata(Hd_dt, 'v');
disp('Digital B coefficients:');
disp(num_d);
disp('Digital A coefficients:');
disp(den_d);

% Plot the magnitude of the digital filter frequency response
figure;
[h, w] = freqz(num_d, den_d, 1024, fs);
plot(w, abs(h));
xlabel('Frequency (Hz)');
ylabel('Magnitude');
title('Magnitude Response of the Digital Filter');
grid on;
