function Test(File1, File2)
    clc;
  
    disp("Cargando " + File1);
    Original = audioread(File1);
    disp("Reproduciendo Archivo Original");
    sound(Original);
    
    pause(12);
    
    disp(" ");
    disp("Cargando " + File2);
    Dec = audioread(File2);
    disp("Reproduciendo Archivo Decodificado");
    sound(Dec);
    
end