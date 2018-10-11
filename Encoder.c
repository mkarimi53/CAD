#include <time.h>
#include "Coding.h"

// image data type
typedef struct image_struct {
   int Rows, Columns;
   double *Pixel_Data;
} image;

// coefficient matrix for DCT
double DCT_Coeffs[8][8];

// function prototypes
void Fetch_Image(char *, image *);
void Discrete_Cosine_Transform(image *, int, double *****, double ***);
void Overall_Quantisation(image *, image *, double *****, double ***, int);
void Init_DCT_Coeffs(void);
static void Fetch_Block_Main(double *, double [][8], int, int, int, int, int);
static void Fetch_Block(double *, double *****, int, int, int, int, int);
void Block_DCT(double *****, double ***, int, int, int);
void Block_Quantisation(double *****, double ***, int, int, int, int);
static void Write_Block(double *****, double *, int, int, int, int, int);
void Lossless_Coding(image *, char *, int);
void Write_Coded_Block(double [][8], FILE *);
void Write_Bits(FILE *, int, int);
void colourSpaceConversion(image *, double *);
void Downsampling(image *, double *, image *);
void Encoder(char *Source_Filename, int Compression_Format, char *Destination_Filename)
{
   image Source_Image, Downsampled_Image, DCT_Image;
    double *Source_Data;
    double *****Block_Data;
    double ***s;
    int colour, i, j, DCT_Rows, DCT_Columns, Block_Rows, Block_Columns;

   strcat(Source_Filename, ".ppm");
   strcat(Destination_Filename, ".mic3");
   printf("Encoding image %s to file %s\n", Source_Filename, Destination_Filename);

   // Compress the image
   Fetch_Image(Source_Filename, &Source_Image);

   //Stage1
   Source_Data = Source_Image.Pixel_Data;
   colourSpaceConversion(&Source_Image, Source_Data);

   //Stage2
   Downsampling(&Source_Image, Source_Data, &Downsampled_Image);

   DCT_Rows = Downsampled_Image.Rows;
   DCT_Columns = Downsampled_Image.Columns;

   Block_Rows = DCT_Rows/8;
   Block_Columns = DCT_Columns/8;

   Block_Data = (double *****)malloc(3*sizeof(double ****));
   s = (double ***)malloc(3*sizeof(double **));
   for (colour = 0; colour < 3; colour++) {
       Block_Data[colour] = (double ****)malloc(Block_Rows*sizeof(double ***));
       s[colour] = (double **)malloc(Block_Rows*sizeof(double *));
       for (i = 0; i < Block_Rows; i++){
          Block_Data[colour][i] = (double ***)malloc(Block_Columns*sizeof(double **));
          s[colour][i] = (double *)malloc(Block_Columns*sizeof(double));
         for (j = 0; j < Block_Columns; j++) {
             Block_Data[colour][i][j] = (double **)malloc(8*sizeof(double *));
             for (int m = 0; m < 8; m++) {
                Block_Data[colour][i][j][m] = (double *)malloc(8*sizeof(double));
             }
         }
     }
 }

   //Stage3
   Discrete_Cosine_Transform(&Downsampled_Image, Compression_Format, Block_Data, s);

   //Stage4
   Overall_Quantisation(&Downsampled_Image, &DCT_Image, Block_Data, s, Compression_Format);

   //Stage5
   Lossless_Coding(&DCT_Image, Destination_Filename, Compression_Format);

   free(DCT_Image.Pixel_Data);
   free(Downsampled_Image.Pixel_Data);
   free(Source_Image.Pixel_Data);
}

void Fetch_Image(char *Filename, image *Source_Image)
{
   int i, j, Rows, Columns;
   char temp_string[20];
   double *Pixel_Data;
   FILE *Source_File;

   char* resultFilename = "Fetch_Image.txt";
   FILE* result;

   clock_t start, end;
   double cpu_time_used;

   start = clock();
   // open the file
   if ((Source_File = fopen(Filename, "rb")) == NULL) {
      printf("Problem opening source image %s\n", Filename);
      exit(0);
   }

   // extract header information
   fscanf(Source_File, "%s", temp_string);     // image type - usually P6
   fscanf(Source_File, "%d", &Columns);        // Pixel Columns
   fscanf(Source_File, "%d", &Rows);           // Pixel Rows
   fscanf(Source_File, "%s", temp_string);     // max colours - usually 255
   fgetc(Source_File);

   // read the image data
   Pixel_Data = (double *)malloc(Rows*Columns*3*sizeof(double));
   for (i = 0; i < Rows; i++)
      for (j = 0; j < Columns; j++) {
         Pixel_Data[RGB_index(Rows,Columns,i,j,R)] = (double)fgetc(Source_File);
         Pixel_Data[RGB_index(Rows,Columns,i,j,G)] = (double)fgetc(Source_File);
         Pixel_Data[RGB_index(Rows,Columns,i,j,B)] = (double)fgetc(Source_File);
      }
   fclose(Source_File);

   Source_Image->Rows = Rows;
   Source_Image->Columns = Columns;
   Source_Image->Pixel_Data = Pixel_Data;

   end = clock();
   cpu_time_used = ((double) (end - start)) /CLOCKS_PER_SEC;

   // print result to Fetch_Image
   printf("result\n");
   if ((result = fopen(resultFilename, "w")) == NULL) {
      printf("Problem creating %s.\n", resultFilename);
      exit(0);
   }
   fprintf(result, "%d\n", Rows);
   fprintf(result, "%d\n", Columns);
   printf("%d\n", Rows);
   printf("%d\n", Columns);
   for(int i = 0; i < Rows; i++){
     for(int j = 0; j < Columns; j++){
       for(int k = 0; k < 3; k++){
         //printf("%d %d %d\n", i, j, k);
         fprintf(result, "%f\n", Pixel_Data[(i * Columns * 3) + (j * 3) + k]);
       }
     }
   }
   fprintf(result, "%f\n", cpu_time_used);
   fclose(result);
}

void colourSpaceConversion(image *Source_Image, double *Source_Data){
    int i, j, Source_Rows, Source_Columns;
    double Y_val, U_val, V_val, R_val, G_val, B_val;
    double RGB_YUV_matrix[9] = {
       0.257,   0.504,   0.098,
      -0.148,  -0.291,   0.439,
       0.439,  -0.368,  -0.071 };
    Source_Rows = Source_Image->Rows;
    Source_Columns = Source_Image->Columns;

    // Colourspace conversion
    for (i = 0; i < Source_Rows; i++)
       for (j = 0; j < Source_Columns; j++) {
          R_val = Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, R)];
          G_val = Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, G)];
          B_val = Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, B)];

          Y_val = RGB_YUV_matrix[0]*R_val + RGB_YUV_matrix[1]*G_val + RGB_YUV_matrix[2]*B_val;
          Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, G)] = Y_val + 16.0;

          U_val = RGB_YUV_matrix[3]*R_val + RGB_YUV_matrix[4]*G_val + RGB_YUV_matrix[5]*B_val;
          Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, B)] = U_val + 128.0;

          V_val = RGB_YUV_matrix[6]*R_val + RGB_YUV_matrix[7]*G_val + RGB_YUV_matrix[8]*B_val;
          Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, R)] = V_val + 128.0;
       }
}

void Downsampling(image *Source_Image, double *Source_Data, image *Downsampled_Image){
    int i, j, Downsampled_Rows, Downsampled_Columns, Source_Rows, Source_Columns;
    int jm5, jm3, jm1, jp1, jp3, jp5;
    double *Downsampled_Data;
    // Downsampling
    Source_Rows = Source_Image->Rows;
    Source_Columns = Source_Image->Columns;
    Downsampled_Rows = Source_Image->Rows;
    Downsampled_Columns = Source_Image->Columns;
    Downsampled_Data = (double *)malloc(Downsampled_Rows*Downsampled_Columns*2*sizeof(double));

    for (i = 0; i < Downsampled_Rows; i++)
       for (j = 0; j < Downsampled_Columns; j++) {
          Downsampled_Data[YUV_index(Downsampled_Rows, Downsampled_Columns, i, j, Y)] =
             Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, G)];
          if (j%2 == 0) {
             jm5 = (j < 5) ? 0 : j - 5;
             jm3 = (j < 3) ? 0 : j - 3;
             jm1 = (j < 1) ? 0 : j - 1;
             jp1 = (j < (Downsampled_Columns - 1)) ? j + 1 : Downsampled_Columns - 1;
             jp3 = (j < (Downsampled_Columns - 3)) ? j + 3 : Downsampled_Columns - 1;
             jp5 = (j < (Downsampled_Columns - 5)) ? j + 5 : Downsampled_Columns - 1;

             Downsampled_Data[YUV_index(Downsampled_Rows, Downsampled_Columns, i, j/2, U)] =
                0.043 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jm5, B)] -
                0.102 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jm3, B)] +
                0.311 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jm1, B)] +
                0.500 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, B)] +
                0.311 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jp1, B)] -
                0.102 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jp3, B)] +
                0.043 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jp5, B)];
             Downsampled_Data[YUV_index(Downsampled_Rows, Downsampled_Columns, i, j/2, V)] =
                0.043 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jm5, R)] -
                0.102 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jm3, R)] +
                0.311 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jm1, R)] +
                0.500 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, j, R)] +
                0.311 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jp1, R)] -
                0.102 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jp3, R)] +
                0.043 * Source_Data[RGB_index(Source_Rows, Source_Columns, i, jp5, R)];
          }
       }

    Downsampled_Image->Rows = Downsampled_Rows;
    Downsampled_Image->Columns = Downsampled_Columns;
    Downsampled_Image->Pixel_Data = Downsampled_Data;
}

void Discrete_Cosine_Transform(image *Downsampled_Image, int Compression_Format, double *****Block_Data, double ***s)
{
   int colour, i, j, DCT_Rows, DCT_Columns, Block_Rows, Block_Columns;
   double *Downsampled_Data;
   DCT_Rows = Downsampled_Image->Rows;
   DCT_Columns = Downsampled_Image->Columns;

   Block_Rows = DCT_Rows/8;
   Block_Columns = DCT_Columns/8;

   Downsampled_Data = Downsampled_Image->Pixel_Data;

   Init_DCT_Coeffs();

   // process the blocks in sequence, from Y to U to V
   // for a given component, process the blocks by rows
   for (colour = 0; colour < 3; colour++) {
      for (i = 0; i < Block_Rows; i++)
         for (j = 0; j < Block_Columns; j++) {
            Fetch_Block(Downsampled_Data, Block_Data, i, j, DCT_Rows, DCT_Columns, colour);
            Block_DCT(Block_Data, s, i, j, colour);
         }
      if (colour == Y) Block_Columns /= 2;   // since U and V have half as many columns
   }
}

void Overall_Quantisation(image *Downsampled_Image, image *DCT_Image, double *****Block_Data, double ***s, int Compression_Format){
    int colour, i, j, DCT_Rows, DCT_Columns, Block_Rows, Block_Columns;
    double *DCT_Data;
    DCT_Rows = Downsampled_Image->Rows;
    DCT_Columns = Downsampled_Image->Columns;

    Block_Rows = DCT_Rows/8;
    Block_Columns = DCT_Columns/8;

    DCT_Data = (double *)malloc(DCT_Rows*DCT_Columns*2*sizeof(double));

    // process the blocks in sequence, from Y to U to V
    // for a given component, process the blocks by rows
    for (colour = 0; colour < 3; colour++) {
       for (i = 0; i < Block_Rows; i++)
          for (j = 0; j < Block_Columns; j++) {
             Block_Quantisation(Block_Data, s, Compression_Format, i, j, colour);
             Write_Block(Block_Data, DCT_Data, i, j, DCT_Rows, DCT_Columns, colour);
          }
       if (colour == Y) Block_Columns /= 2;   // since U and V have half as many columns
    }

    DCT_Image->Rows = DCT_Rows;
    DCT_Image->Columns = DCT_Columns;
    DCT_Image->Pixel_Data = DCT_Data;
}

void Init_DCT_Coeffs(void)
{
   int i, j;
   double s;

   for (i = 0; i < 8; i++) {
      s = (i == 0) ? sqrt(1.0/8.0) : sqrt(2.0/8.0);
      for (j = 0; j < 8; j++)
         DCT_Coeffs[i][j] = s * cos((PI / 8.0) * i * (j + 0.5));
   }
}

static void Fetch_Block(double *Downsampled_Data, double ***** Block_Data,
   int Block_Row, int Block_Column, int Rows, int Columns, int colour)
{
   int i, j;

   for (i = 0; i < 8; i++)
      for (j = 0; j < 8; j++)
         Block_Data[colour][Block_Row][Block_Column][i][j] = Downsampled_Data[YUV_index(Rows, Columns,
            8*Block_Row+i, 8*Block_Column+j, colour)];
}

static void Fetch_Block_Main(double *Downsampled_Data, double Block_Data[][8],
   int Block_Row, int Block_Column, int Rows, int Columns, int colour)
{
   int i, j;

   for (i = 0; i < 8; i++)
      for (j = 0; j < 8; j++)
         Block_Data[i][j] = Downsampled_Data[YUV_index(Rows, Columns,
            8*Block_Row+i, 8*Block_Column+j, colour)];
}

void Block_DCT(double *****Block_Data, double ***s, int Row, int Column, int Color)
{
   int i, j, k;
   double temp[8][8];

   // post-multiplication with the transposed coefficient matrix
   for (i = 0; i < 8; i++)
      for (j = 0; j < 8; j++) {
         s[Color][Row][Column] = 0.0;
         for (k = 0; k < 8; k++)
            s[Color][Row][Column] += Block_Data[Color][Row][Column][i][k] * DCT_Coeffs[j][k];
         temp[i][j] = s[Color][Row][Column];
      }

   // pre-multiplication with the coefficient matrix
   for (j = 0; j < 8; j++)
      for (i = 0; i < 8; i++) {
         s[Color][Row][Column] = 0.0;
         for (k = 0; k < 8; k++)
            s[Color][Row][Column] += DCT_Coeffs[i][k] * temp[k][j];
         Block_Data[Color][Row][Column][i][j] = s[Color][Row][Column];
      }
}

void Block_Quantisation(double *****Block_Data, double ***s, int Compression_Format, int Row, int Column, int Color)
{
    int i, j, k;
    // quantization
    for (j = 0; j < 8; j++)
       for (i = 0; i < 8; i++) {
          if (Compression_Format == 0) {      // use quantization matrix Q0
             if ((i + j) >= 8) s[Color][Row][Column] = 64.0;
             else if ((i + j) >= 6) s[Color][Row][Column] = 32.0;
             else if ((i + j) >= 4) s[Color][Row][Column] = 16.0;
             else if ((i + j) >= 2) s[Color][Row][Column] = 8.0;
             else if ((i + j) >= 1) s[Color][Row][Column] = 4.0;
             else s[Color][Row][Column] = 8.0;
          } else {                            // use quantization matrix Q1
             if ((i + j) >= 8) s[Color][Row][Column] = 16.0;
             else if ((i + j) >= 6) s[Color][Row][Column] = 8.0;
             else if ((i + j) >= 4) s[Color][Row][Column] = 4.0;
             else if ((i + j) >= 2) s[Color][Row][Column] = 2.0;
             else if ((i + j) >= 1) s[Color][Row][Column] = 2.0;
             else s[Color][Row][Column] = 8.0;
          }

          // pointwise division
          s[Color][Row][Column] = Block_Data[Color][Row][Column][i][j] / s[Color][Row][Column];

          // clipping to retain 9-bit coefficients (-256 .. 255)
          Block_Data[Color][Row][Column][i][j] = (s[Color][Row][Column] < -256.0) ? -256.0 : (s[Color][Row][Column] > 255.0) ? 255.0 : s[Color][Row][Column];
       }
}

static void Write_Block(double *****Block_Data, double *DCT_Data,
   int Block_Row, int Block_Column, int Rows, int Columns, int colour)
{
   int i, j;

   for (i = 0; i < 8; i++)
      for (j = 0; j < 8; j++)
         DCT_Data[YUV_index(Rows, Columns, 8*Block_Row+i, 8*Block_Column+j, colour)] =
            Block_Data[colour][Block_Row][Block_Column][i][j];
}

void Lossless_Coding(image *DCT_Image, char *Filename, int Compression_Format)
{
   int colour, i, j, DCT_Rows, DCT_Columns, Block_Rows, Block_Columns;
   double *DCT_Data, Block_Data[8][8];
   FILE *Destination_File;

   // Open the file
   if ((Destination_File = fopen(Filename, "wb")) == NULL) {
      printf("Problem opening destination compressed stream %s\n", Filename);
      exit(0);
   }

   DCT_Rows = DCT_Image->Rows;
   DCT_Columns = DCT_Image->Columns;

   Block_Rows = DCT_Rows/8;
   Block_Columns = DCT_Columns/8;

   DCT_Data = DCT_Image->Pixel_Data;

   // provide the compressed stream header
   fprintf(Destination_File, "%c", 0x0A);
   fprintf(Destination_File, "%c", ((DCT_Rows >> 8) & 0x3F) | (Compression_Format << 6));
   fprintf(Destination_File, "%c", DCT_Rows & 0xFF);
   fprintf(Destination_File, "%c", DCT_Columns >> 8);
   fprintf(Destination_File, "%c", DCT_Columns & 0xFF);

   // process the blocks in sequence
   for (colour = 0; colour < 3; colour++) {
      for (i = 0; i < Block_Rows; i++)
         for (j = 0; j < Block_Columns; j++) {
            Fetch_Block_Main(DCT_Data, Block_Data, i, j, DCT_Rows, DCT_Columns, colour);
            Write_Coded_Block(Block_Data, Destination_File);
         }
      if (colour == Y) Block_Columns /= 2;
   }

   // pad with zeros to the end of a 16 bit word
   Write_Bits(Destination_File, 0, 16);
   fclose(Destination_File);
}

void Write_Coded_Block(double Block_Data[][8], FILE *Destination_File)
{
   int i, j, temp, Scanned_Block[64];
   double s;

   // round double precision values to integers
   for (i = 0; i < 64; i++) {
      j = Scan_Pattern[i];
      s = Block_Data[j/8][j%8];
      s = s + ((s > 0) ? 0.5 : -0.5);
      Scanned_Block[i] = (int)s;
   }

   // losslessly code the block
   i = 0; while (i < 64) {
      j = 0; while ((i + j < 64) && (Scanned_Block[i+j] == 0)) { j++; }
      if (i + j < 64) {
         if (j > 0) {
            temp = j;
            while (temp >= 8) {
               Write_Bits(Destination_File, (ZERO_RUN << 3), 5);
               temp -= 8;
            }
            if (temp > 0)
               Write_Bits(Destination_File, ((ZERO_RUN << 3) | temp), 5);
         }
         if ((Scanned_Block[i+j] <= 15) && (Scanned_Block[i+j] >= -16))
            Write_Bits(Destination_File, ((CODE_5 << 5) | (Scanned_Block[i+j] & 0x1F)), 7);
         else Write_Bits(Destination_File, ((CODE_9 << 9) | (Scanned_Block[i+j] & 0x1FF)), 11);
      } else Write_Bits(Destination_File, BLOCK_END, 2);
      i += j + 1;
   }
}

void Write_Bits(FILE *Destination_File, int bits, int length)
{
   static unsigned int buffer = 0, pointer = 0;

   buffer = (buffer << length) | bits;
   pointer += length;

   while (pointer >= 8) {
      fprintf(Destination_File, "%c", 0xFF & (buffer >> (pointer - 8)));
      pointer -= 8;
   }
}
