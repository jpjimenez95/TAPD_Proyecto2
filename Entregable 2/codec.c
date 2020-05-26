#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>
#include <time.h>


#define PI 3.1415926536

typedef struct {
	int64_t re;
	int64_t im;
} double_complex_t;

int init  = 22;
int N = 64;
int EXP = 6;

/* For the reference (double) FFT */


/*void fft_init(double_complex_t *W, unsigned short EXPP)                 //para punto flotante nada mas
{
    unsigned short L,LE,LE1;

    for (L=1; L<=EXP; L++)
    {
        LE=1<<L;                        // LE=2^L=points of sub DFT
        LE1=LE>>1;     	                // Number of butterflies in sub-DFT
        W[L-1].re = cos(PI/LE1);
        W[L-1].im = -sin(PI/LE1);
    }
}*/

void convertir_to_complex(short* buffer_short, double_complex_t* buffer_complex, int len)
{
    int i;
    for (i = init; i < len; i++){

        buffer_complex[i].re = (int)buffer_short[i];
        buffer_complex[i].im = 0;

        //printf("Num %d: %d + %di\n",i,buff_complex[i].re, buff_complex[i].im);
    }
    init = 0;
}

float fix_to_fload(int x, int e){
    // f es el numero en punto fijo de tipo int
    // e es la cantidad de bits que se le dio la seccion decimal
    double f;
    int sign;
    int c;

    c = abs(x);
    sign = 1;
    
    if (x < 0){
    /* Las siguientes 3 lineas es para devolverlo del complemento a 2 
        si el numero original era negativo */ 

        c = x - 1; 
        c = ~c;
        sign = -1;
    }

    /*Lo que se hace es simplemente multiplicar el numero 
    en punto fijo dividido por 2 elevado a la e*/
    f = (1.0 * c)/(1<<e);
    f = f * sign;
    return f;
}

int float_to_fix(double f, int e){
    // f es el numero original de tipo double
    // e es la cantidad de bits que se le va a dar a la seccion decimal

    /*Lo que se hace es simplemente multiplicar el numero 
    original multiplicado por 2 elevado a la e*/
    double a = f*(1<<e);

    int b = (int)(round(a));
    
    if (a < 0){
        /* Las siguientes 3 lineas es para pasarlo a complemento a 2 
        si el numero original es negativo */ 

        b = abs(b);
        b = ~b;
        b = b + 1;
    }
    return b;
}

void char_to_complex(char* buff_char, double_complex_t* buff_complex, int len){

    double puente_re[len - 2];
    double puente_im[len - 2];

    puente_re[0] = fix_to_fload(buff_char[0],8);
    puente_im[0] = fix_to_fload(buff_char[1],8);

    puente_re[(len-1)/2] = fix_to_fload(buff_char[len-2],8);
    puente_im[(len-1)/2] = fix_to_fload(buff_char[len-1],8);

    for (int i = 2; i < len-2; i = i+2){
        puente_re[i/2] = fix_to_fload(buff_char[i],8);
        puente_im[i/2] = fix_to_fload(buff_char[i + 1],8);

        puente_re[len-i/2-2] =   fix_to_fload(buff_char[i],8);
        puente_im[len-i/2-2] = - fix_to_fload(buff_char[i + 1],8);
    }

    for (int i = 0; i < len-2; i++){
        buff_complex[i].re = float_to_fix(puente_re[i],15);
        buff_complex[i].im = float_to_fix(puente_im[i],15);
    }
    /*
    for (int j = 0; j < len - 2; j++){
        printf("Num %d: %f + %fi\n",j,creal(buff_complex[j]), cimag(buff_complex[j]));
    }*/
}

void save_fft(double_complex_t* buff_complex, int len){         //
    FILE * file;
    file = fopen ("seno400_zip.wav", "a");

    char re;
    char im;
    double puente_re;
    double puente_im;
    
    char* progbar="-/|\\";
    int progidx=0;

    for (int i=0; i<len/2+1;i++){

        puente_re = fix_to_fload(buff_complex[i].re,15); 
        puente_im = fix_to_fload(buff_complex[i].im,15);

        if (puente_re > 0.5){
            puente_re = 0.5;
        }
        else if (puente_re < -0.5){
            puente_re = -0.5;
        }

        if (puente_im > 0.5){
            puente_im = 0.5;
        }
        else if (puente_im < -0.5){
            puente_im = -0.5;
        }

        re = (char)float_to_fix(puente_re,8);
        im = (char)float_to_fix(puente_im,8);
        fwrite(&re, sizeof(char), 1, file);
        fwrite(&im, sizeof(char), 1, file);

        printf("%c\r",progbar[progidx++&3]);
        fflush(stdout);

        //printf("0x%04x = 0x%08lx -- %.5f = %.5f\n"  ,re,buff_complex[i].re,(double)re/(1<<8),fix_to_fload(buff_complex[i].re,15));
        //printf("0x%04x = 0x%08lx -- %.5f = %.5f\n\n",im,buff_complex[i].im,(double)im/(1<<8),fix_to_fload(buff_complex[i].im,15));
    }

    fclose(file); 
}

void save_csv(double_complex_t* buff_complex, int len){
    FILE * file;
    file = fopen ("deco.csv", "a");

    char* progbar="-/|\\";
    int progidx=0;

    for (int i=0; i<len; i++){
        fprintf(file, "%f\n", (double)buff_complex[i].re/(1<<15));
        printf("%c\r",progbar[progidx++&3]);
        fflush(stdout);
    }

    fclose(file); 
}

void save_wav(double_complex_t *buff_complex, int len){
    FILE * file;
    file = fopen ("deco.wav", "a");
    
    short var;
    double puente;

    char* progbar="-/|\\";
    int progidx=0;

    for (int i=0; i<len;i++){
        
        //Por razones que escapan a mi entendimiento, se debe hacer el paso a flotante 
        //y luego tirarlo de vuelta a punto fijo, esto es un misterio, pero funciona
        puente = fix_to_fload(buff_complex[i].re,15);
        var    = (short)float_to_fix(puente,15);;
        fwrite(&var, sizeof(short), 1, file);
    }

    fclose(file); 
}

void fixp_fft_init(double_complex_t *W, unsigned short EXPP, int fix)   //inicializa para punto fijo
{
	  unsigned short L,LE,LE1;
      double re, im;

	  for (L=1; L<=EXP; L++)       // Create twiddle factor table
	  {
	    LE=1<<L;                   // LE=2^L=points of sub DFT
	    LE1=LE>>1;     	           // Number of butterflies in sub-DFT
        re = cos(PI/LE1);
        im = sin(PI/LE1);
      
	    W[L-1].re = re*(1<<fix);
	    W[L-1].im = im*(1<<fix);
	  }
}

void fft_bit_rev(double_complex_t *X, short EXPP)
{
    unsigned short i,j,k;
    unsigned short NN=1<<EXP;              // Number of points for FFT
    unsigned short N2=N>>1;
    double_complex_t  temp;	            // Temporary storage of the complex variable

    for (j=0,i=1;i<N-1;i++)
    {
        k=N2;
        while(k<=j)
        {
            j-=k;
            k>>=1;
        }
        j+=k;

        if (i<j)
        {
            temp = X[j];
            X[j] = X[i];
            X[i] = temp;
        }
    }
}

void fft(double_complex_t *X, unsigned short EXPP, double_complex_t *W, unsigned short SCALE, int fix)
{
    double_complex_t temp;

    double_complex_t U;

    unsigned short i,j;
    unsigned short id;                  // Indice del punto mas bajo en la mariposa  
    unsigned short NN=1<<EXP;           // Numero de puntos para FFT  
    unsigned short L;                   // Estado de la FFT
    unsigned short LE;                  // Numero de puntos en el sub DFT del estado L
                                        // y offset para el siguiente estado en la DFT 
    unsigned short LE1;                 // Numero de mariposas en una DFT en el estado L
                                        // Tambien es offset el punto mas bajo
                                        // en la mariposa del estado L
    int64_t scale;

    /* DEBUG */
    /*printf("Inputs\n");
    for (j=0; j<N; j++)
    {
        printf("%f + j*%f\n", X[j].re, X[j].im);
    }*/

    // Arrange X[] in bit-reverse order
    //fft_bit_rev(X, EXP);

    /* DEBUG */
    /*printf("Inputs after reversal\n");
    for (j=0; j<N; j++)
    {
        printf("%f + j*%f\n", X[j].re, X[j].im);
    }*/

    scale = 0.5*(1<<fix);     
    if (SCALE == 0)  
    {       
        scale = (1<<fix);
    }    
    for (L=1; L<=EXP; L++)              // Mariposa FFT
    {
        LE=1<<L;                        // LE=2^L=points of sub DFT
        LE1=LE>>1;                      // Numero de mariposas en una sub DFT

        U.re = 1.0*(1<<fix);
        U.im = 0.;

        for (j=0; j<LE1;j++)
        {
            for(i=j; i<N; i+=LE)            // Se hacen las mariposas
            {
                id=i+LE1;
                temp.re = (X[id].re*U.re - X[id].im*U.im)/(1<<fix);
                temp.re = temp.re*scale/(1<<fix);

                temp.im = (X[id].im*U.re + X[id].re*U.im)/(1<<fix);
                temp.im = temp.im*scale/(1<<fix);

                X[id].re = X[i].re*scale/(1<<fix) - temp.re;
                X[id].im = X[i].im*scale/(1<<fix) - temp.im;

                X[i].re = X[i].re*scale/(1<<fix) + temp.re;
                X[i].im = X[i].im*scale/(1<<fix) + temp.im;
            }
                
            // Recursive compute W^k as U*W^(k-1)
            temp.re = U.re*W[L-1].re/(1<<fix) - U.im*W[L-1].im/(1<<fix);
            U.im    = U.re*W[L-1].im/(1<<fix) + U.im*W[L-1].re/(1<<fix);
            U.re = temp.re;
            }

            /* DEBUG */
            /*printf("Stage %d\n", L);
            for (j=0; j<N; j++)
            {
            printf("%f + j*%f\n", X[j].re, X[j].im);
            }*/

    }
    
}

void ifft(double_complex_t *X, unsigned short EXPP, double_complex_t *W, unsigned short SCALE, int Fix){

    unsigned short NN  = (1<<EXP); /* Number of points for FFT */
    //fft_bit_rev(X, EXP);

    for(int i=0;i<(1<<EXP);i++){        

        X[i].re =  X[i].re*NN;
        X[i].im = -X[i].im*NN;
    }

    fft(X, EXP, W, SCALE,Fix);
/*
    for(int i=0;i<N;i++){
        //printf("Num %d: %ld + %ld i\n",i,X[i].re,X[i].im);
        printf("Num %d: %f + %f i\n",i,(double)X[i].re/(1<<Fix),(double)X[i].im/(1<<Fix));
    }
*/
}

void main (void)
{
    unsigned short i;
    FILE *archivo, *comprimido;
    int n;
    double_complex_t *buffer_Complex, *twiddle_table_dblcplx, *buffer_ComplexIFFT;
    short *buffer_Short;
    char *buffer_ShortIFFT;

    buffer_ShortIFFT = malloc((int)66 * sizeof(char));
    buffer_ComplexIFFT = malloc((int)64    * sizeof(double_complex_t));

    buffer_Complex    = malloc((int)N * sizeof(double_complex_t));
    buffer_Short    = malloc((int)N * sizeof(short));
    twiddle_table_dblcplx = malloc((int)EXP * sizeof(double_complex_t));/* Temp storage for twiddle factors */
    /*if (archivo != NULL)
    {
        fseek(archivo,44,SEEK_SET);
        fread(audio,2,64000,archivo);
        fclose(archivo);
        //n=2*ceil(n/2.0);
        //for (n=0; n<400; n++) printf("%d\n", audio[n][0]);
    }
    else printf("\nFalla al leer el archivo.\n");*/

    fixp_fft_init(twiddle_table_dblcplx, EXP, 15);
    
	/* Print the twiddle factors */
	/*printf("Twiddle factors:\n");
	for (i=0; i<EXP; i++)
	{
		printf("%f + j*%f\n", twiddle_table_dblcplx[i].re, twiddle_table_dblcplx[i].im);
	}*/

    ///////////////////////////////////
    //          Codificador          //
    ///////////////////////////////////

	/* This FFT needs complex inputs */
    comprimido = fopen("seno400_zip.wav","w");
    fclose(comprimido);

    archivo = fopen("seno400.wav","r");
    
    if(archivo != NULL)
    {
        size_t byte_leido;
        printf("Generando archivo comprimido.\n");
        while(byte_leido > 0)
        {
            byte_leido = fread(buffer_Short, sizeof(short), N, archivo);
            convertir_to_complex(buffer_Short, buffer_Complex, byte_leido);
            
            fft_bit_rev(buffer_Complex, EXP);
            printf("paso el fft_bit_rev");
            fft(buffer_Complex, EXP, twiddle_table_dblcplx, 1, 15);      //como procesar bloques de 64 muestras
            save_fft(buffer_Complex, N);
            

        }
        fclose(archivo);
    }
    else
    {
        printf("Archivo vacio.\n");
    }
    
    


    ////////////////////////////////////
    //           Decodificador        //
    ////////////////////////////////////

    FILE *IFFT1, *IFFT2;
	IFFT1 = fopen("deco.csv", "w");
    fclose(IFFT1);

    IFFT2 = fopen("deco.wav", "w");
    fclose(IFFT1);

    fixp_fft_init(twiddle_table_dblcplx, EXP, 15);

    archivo = fopen("seno400_zip.wav", "r");

    if (archivo != NULL)
    {
        size_t byte_leido;
        printf("Descomprimiendo archivo.\n");
        while (byte_leido > 0)
        {
            byte_leido = fread(buffer_ShortIFFT, sizeof(char), 66, archivo);
            char_to_complex(buffer_ShortIFFT, buffer_ComplexIFFT, byte_leido);
            if (byte_leido > 0)
            {
                fft_bit_rev(buffer_ComplexIFFT, EXP);
                ifft(buffer_ComplexIFFT, EXP, twiddle_table_dblcplx, 1,15);
                save_csv(buffer_ComplexIFFT, 64);
                save_wav(buffer_ComplexIFFT, 64);
            }
            
        }
        printf("Decodificacion finalizada.\n");
        fclose(archivo);
        
    }
    else
    {
        printf("Archivo codificado vacio.\n");
    }
    
    

	
}