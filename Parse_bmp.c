
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Parse_bmp(char *Source_Filename, char *Destination_Filename)
{
   unsigned int i, j, index, Image_Rows, Image_Columns;
   unsigned char temp_char, *Image_Data;
   FILE *Source_File, *Destination_File;

   strcat(Source_Filename, ".bmp");
   strcat(Destination_Filename, ".ppm");
   printf("Parsing file %s to %s\n", Source_Filename, Destination_Filename);

   // open files
   if ((Source_File = fopen(Source_Filename, "rb")) == NULL) {
      printf("Problem with file %s\n", Source_Filename); exit(0); }
   if ((Destination_File = fopen(Destination_Filename, "wb")) == NULL) {
      printf("Problem with file %s\n", Destination_Filename); exit(0); }

   // strip header beginning
   for (i = 0; i < 18; i++)
      fgetc(Source_File);

   // retrieve image width
   Image_Columns = 0;
   for (i = 0; i < 4; i++) {
      temp_char = fgetc(Source_File);
      Image_Columns += temp_char << (8 * i);
   }
   printf("Image_Columns: %lu\n", Image_Columns);

   // retrieve image height
   Image_Rows = 0;
   for (i = 0; i < 4; i++) {
      temp_char = fgetc(Source_File);
      Image_Rows += temp_char << (8 * i);
   }
   printf("Image_Rows: %lu\n", Image_Rows);

   // strip header ending
   for (i = 0; i < 28; i++)
      fgetc(Source_File);

   // memory allocation
   Image_Data = (unsigned char *)malloc(3*Image_Rows*Image_Columns*sizeof(unsigned char));

   // read image
   for (i = 0; i < Image_Rows; i++)
      for (j = 0; j < Image_Columns; j++) {
         index = (3*(((Image_Rows)-(i)-1)*(Image_Columns)+(j)));
         Image_Data[index+2] = fgetc(Source_File); // B
         Image_Data[index+1] = fgetc(Source_File); // G
         Image_Data[index+0] = fgetc(Source_File); // R
      }

   // write image
   fprintf(Destination_File, "P6\n%d %d\n255\n", Image_Columns, Image_Rows);
   for (i = 0; i < Image_Rows; i++)
      for (j = 0; j < Image_Columns; j++) {
         index = (3*((i)*(Image_Columns)+(j)));
         fprintf(Destination_File, "%c", Image_Data[index+0]);  // R
         fprintf(Destination_File, "%c", Image_Data[index+1]);  // G
         fprintf(Destination_File, "%c", Image_Data[index+2]);  // B
      }

   fclose(Source_File);
   fclose(Destination_File);

   free(Image_Data);
}
