function Codec()
    %CreateWav('Sine500Hz.wav', 500);
    CreateWav('Random2.wav', 300);
    [Original, Fs] = audioread('Random2.wav');
    Original = transpose(Original);
    
    % Codifica la senal de original
    for i = 1:64:(length(Original)-1)
        Samples = Original(i:(i+63));
        if i == 1
            Codificado = RFFT(Samples);
        else
            Temp = RFFT(Samples);
            Codificado = [Codificado Temp]; 
        end
    end
    
    % Guarda los datos codificados
    fileID = fopen('Random2.bin', 'w');
    fwrite(fileID, Codificado, 'int8');
    fclose(fileID);
    
    % Decodifica la senal obtenida en el paso anterior
    for j = 1:33:(length(Codificado)-1)
        Samples = Codificado(j:(j+32));
        if j == 1
            Decodificado = IRFFT(Samples);
        else
            Temp = IRFFT(Samples);
            Decodificado = [Decodificado Temp]; 
        end
    end
    
    %Original(80001) = [];
    t = (0:79999)/8000;
    
    % Graficacion de las dos senales
    subplot(2,1,1)
    plot(t ,Original)
    title('Senal Original')
    xlabel('Tiempo [s]')
    ylabel('Amplitud')
    xlim([0 16e-3])
    
    subplot(2,1,2)
    plot(t, Decodificado)
    title('Senal Decodificada')
    xlabel('Tiempo [s]')
    ylabel('Amplitud')
    xlim([0 16e-3])
    
end

function CreateWav(FileName, Freq)
    Fs = 8e3;   % Frecuencia de muestreo 8kHz
    sample = zeros(1,125);
    
    % Creacion de tonos puros
    for i = 1:80001
        y = sin(2*pi*Freq*i/Fs);
        sample(i) = half(y);
    end
    
    %{
    % Creacion del tono distorisonado con 3 armonicas
    for i = 1:80001
        y = sin(2*pi*Freq*i/Fs)-0.4*sin(4*pi*Freq*i/Fs)+0.6*sin(6*pi*Freq*i/Fs);
        sample(i) = half(y);
    end
    %}
    
    %{ 
    %Codigo para crear ejemplos mas aleatorios
    for i = 1:4:(80001-1)
        % Codigo para crear ejemplos mas aleatorios
        y1 = sin(2*pi*Freq*i/Fs)-0.4*sin(4*pi*Freq*i/Fs);
        y2 = -0.4*sin(4*pi*Freq*i/Fs)+0.6*sin(6*pi*Freq*i/Fs);
        y3 = (-1)^i;
        y4 = 0.2;
        sample(i) = half(y1);
        sample(i+1) = half(y2);
        sample(i+2) = half(y3);
        sample(i+3) = half(y4);
        
    end
    %}
    
    % Creacion del archivo wave
    audiowrite(FileName, sample, Fs);
end

function X = RFFT(Samples)
    n = length(Samples);    % Cantidad total de muestras
    n2 = n/2;
    
    % Creacion de una senal compleja con las compoenentes pares e impares
    % de la senal de entrada
    z = Samples(1:2:n) + 1j*Samples(2:2:n);
    
    Z = fft(z);
    disp(Z(1));
    disp(Z(n2:-1:2));
    Ze = 0.5*( Z + conj([Z(1),Z(n2:-1:2)]));        % even part
    Zo = -0.5*1j*( Z - conj([Z(1),Z(n2:-1:2)]) );     % odd part
    X = [Ze,Ze(1)] + exp(-1j*2*pi/n*(0:n2)).*[Zo,Zo(1)];  % combine
    
end

function x = IRFFT(Samples)
    n = 2 * (length(Samples) - 1 ); % Cantidad de muestras
    s = length(Samples) - 1;
      
    xn = zeros(1,n); % Variable temporal
    xn(1:length(Samples)) = Samples;
    xn(length(Samples)+1:n) = conj(Samples(s:-1:2));
    x = real(ifft(xn));
end

