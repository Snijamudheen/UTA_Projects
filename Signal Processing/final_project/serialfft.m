clear all;

fs = 625;
TimeInterval = 1/fs;
N = 256;
fscale = linspace(-fs/2,fs/2,N);
A = [1.0000   -1.5274    0.9673   -0.2141];
B = [0.0282    0.0846    0.0846    0.0282];
[H,W]=freqz(B,A,N/2);
H2 = [fliplr(H.') H.'];
W = [-fliplr(W.') W.'];
W = (W*fs/2)./pi;

while(1)
    
    SerialPort='com4'; %serial port
    s = serial(SerialPort,'BaudRate', 115200);
    fopen(s);
   
    for i = 1:N+1

        %str = fscanf(s);
        x(i)=str2double(fscanf(s));
        pause(TimeInterval);
    end

    x = x(2:end);

    X = fft(x);
    X = X./max(abs(X));
    Xs = fftshift(abs(X));
    figure(1),plot(W(129:256),abs(H2(129:256)),fscale(129:256),Xs(129:256)),axis([0,fs/2,0,1]),xlabel('Hz'),title('Normalized DFT of Samples'),ylabel('Magnitude');
    figure(2),plot(x),axis([0,N,-1.5,1.5]),title('Time-Domain Plot of Samples');
    clear x;
    
    fclose(s);
    delete(s);
    clear s;
    %instrreset;
end

