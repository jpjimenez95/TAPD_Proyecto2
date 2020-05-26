function Codec()
    %CreateWav('Sine500Hz.wav', 500);
    %CreateWav('Random.wav', 250);
    [Original, Fs] = audioread('Random.wav');
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
    fileID = fopen('Random.bin', 'w');
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
    
    Original(80001) = [];
    t = (0:79999)/8000;
    
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
    % Creacion del tono
    for i = 1:80001
        y = sin(2*pi*Freq*i/Fs)-0.4*sin(4*pi*Freq*i/Fs)+0.6*sin(6*pi*Freq*i/Fs);
        %y = sin(2*pi*Freq*i/Fs); %Tonos Puros
        sample(i) = half(y);
    end
    
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
    n = 2 * (length(Samples) - 1 );
    s = length(Samples) - 1;
      
    xn = zeros(1,n);
    xn(1:length(Samples)) = Samples;
    xn(length(Samples)+1:n) = conj(Samples(s:-1:2));
    x = real(ifft(xn));
end

