
#include "Coding.h"

void Parse_bmp(char *, char *);
void Encoder(char *, int, char *);
void Decoder(char *, char *);
void Compare(char *, char *);

int main(int argc, char *argv[])
{
   int compression_format, debug_level;
   char filename_1[100], filename_2[100];

   // Extract command line parameters
   if (argc > 1) {
      if (!strcmp(argv[1], "-parse")) {
         if (argc != 4) {
            printf("Format for parsing: Project -parse input_file output_file\n");
            printf("   input_file is a .bmp file\n");
            printf("   output_file is a .ppm file\n");
            printf("i.e. \"Project -parse file1 file2\" will parse file1.bmp and produce file2.ppm\n");
         } else {
            sscanf(argv[2], "%s", filename_1);
            sscanf(argv[3], "%s", filename_2);
            Parse_bmp(filename_1, filename_2);
         }
      } else if (!strcmp(argv[1], "-encode")) {
         if (argc != 5) {
            printf("Format for encoding: Project -encode input_file format output_file\n");
            printf("   input_file is a .ppm file\n");
            printf("   format is 0 or 1\n");
            printf("   output_file is a .mic3 file\n");
            printf("i.e. \"Project -encode file1 0 file2\" will compress file1.ppm to file2.mic3\n");
            printf("   using quantization matrix 0\n");
         } else {
            sscanf(argv[2], "%s", filename_1);
            sscanf(argv[3], "%d", &compression_format);
            sscanf(argv[4], "%s", filename_2);
            Encoder(filename_1, compression_format, filename_2);
         }
      }
  } else {
      printf("Usage is as follows:\n\n");

      printf("Format for parsing: Project -parse input_file output_file\n");
      printf("   input_file is a .bmp file\n");
      printf("   output_file is a .ppm file\n");
      printf("i.e. \"Project -parse file1 file2\" will parse file1.bmp and produce file2.ppm\n");
      printf("\n");

      printf("Format for encoding: Project -encode input_file format output_file\n");
      printf("   input_file is a .ppm file\n");
      printf("   format is 0 or 1\n");
      printf("   output_file is a .mic3 file\n");
      printf("i.e. \"Project -encode file1 0 file2\" will compress file1.ppm to file2.mic3\n");
      printf("   using quantization matrix 0\n");
      printf("\n");
   }

   return 0;
}
