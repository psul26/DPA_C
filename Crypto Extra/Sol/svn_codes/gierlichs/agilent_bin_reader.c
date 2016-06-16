//
// Example Program for Reading Binary Data
// The following is a programming example of reading a Binary Data (.bin) file and converting it to an XYPairs (.csv) file without a file header.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//*****************************************************************************
//
//  Description: This file is broken into three sections
//    Section 1: Data Structures to describe Infiniium Public Waveform File
//    Section 2: Functions to correctly read .bin files
//    Section 3: Functions to convert a .bin file to .csv file
//*****************************************************************************
//
//  Description: Structures and Enumerations to describe Infiniium
//               Public Waveform File - using these structures assumes
//               a 32-Bit x86 Compiler
//
typedef struct
{
   char Cookie[2];
   char Version[2];
   int  FileSize;
   int  NumberOfWaveforms;
} tPBFileHeader;
const char PB_COOKIE[2] = {'A', 'G'};
const char PB_VERSION[2] = {'1', '0'};
#define DATE_TIME_STRING_LENGTH 16
#define FRAME_STRING_LENGTH  24
#define SIGNAL_STRING_LENGTH 16
 
typedef struct
{
   int    HeaderSize;
   int    WaveformType;
   int    NWaveformBuffers;
   int    Points;
   int    Count;
   float  XDisplayRange;
   double XDisplayOrigin;
   double XIncrement;
   double XOrigin;
   int    XUnits;
   int    YUnits;
   char   Date[DATE_TIME_STRING_LENGTH];
   char   Time[DATE_TIME_STRING_LENGTH];
   char   Frame[FRAME_STRING_LENGTH];
   char   WaveformLabel[SIGNAL_STRING_LENGTH];
   double TimeTag;
   unsigned int SegmentIndex;
} tPBWaveformHeader;
 
typedef struct
{
   int   HeaderSize;
   short BufferType;
   short BytesPerPoint;
   int   BufferSize;
} tPBWaveformDataHeader;
 
typedef enum
{
   PB_UNKNOWN,
   PB_NORMAL,
   PB_PEAK_DETECT,
   PB_AVERAGE,
   PB_HORZ_HISTOGRAM,
   PB_VERT_HISTOGRAM,
   PB_LOGIC
} ePBWaveformType;
 
typedef enum
{
   PB_DATA_UNKNOWN,
   PB_DATA_NORMAL,
   PB_DATA_MAX,
   PB_DATA_MIN,
   PB_DATA_TIME,
   PB_DATA_COUNTS,
   PB_DATA_LOGIC
} ePBDataType;
 
//*****************************************************************************
//
//  Description: The next set of functions:
//     ReadWaveformHeader
//     ReadWaveformDataHeader
//     ReadLogicWaveform
//     ReadAnalogWaveform
//     ReadHistogramWaveform
//     IgnoreWaveformData
//
//  Demonstrate how to correctly read the Infiniium Public
//  Waveform file with an eye to compatibility with future
//  format changes.
//
// Returns 0 if not sucessful
int ReadWaveformHeader(FILE* inputFile,
                       tPBWaveformHeader* waveformHeader)
{
   char* headerBuffer;
   int   success, headerSize;
   // Assume we'll fail
   success = 0;
   if (waveformHeader)
   {
      // read in header size
      headerSize = 0;
      fread(&headerSize, 1, sizeof(headerSize), inputFile);
      // create header buffer
      headerBuffer = (char*) malloc(headerSize);
      if (headerBuffer)
      {
         // rewind back the headerSize
         fseek(inputFile, - (int)(sizeof(headerSize)), SEEK_CUR);
         // Now read in the entire header
         fread(headerBuffer, 1, headerSize, inputFile);
         // Now set dataHeader from headerBuffer
         // any extra information stored in the file
         // will be ignored
         memcpy((char*) waveformHeader, headerBuffer, sizeof(tPBWaveformHeader));
         success = 1;
         // Just is case WaveformType has been enhanced
         if (waveformHeader->WaveformType > PB_LOGIC)
         {
            waveformHeader->WaveformType = PB_UNKNOWN;
         }
         // Done with headerBuffer
         free(headerBuffer);
      }
   }
   return success;
}
 
// Returns 0 if not sucessful
int ReadWaveformDataHeader(FILE* inputFile,
                           tPBWaveformDataHeader* dataHeader)
{
   char* headerBuffer;
   int   success, headerSize;
   // Assume we'll fail
   success = 0;
   if (dataHeader)
   {
      // read in header size
      headerSize = 0;
      fread(&headerSize, 1, sizeof(headerSize), inputFile);
      // create header buffer
      headerBuffer = (char*) malloc(headerSize);
      if (headerBuffer)
      {
         // rewind back the headerSize
         fseek(inputFile, - (int)(sizeof(headerSize)), SEEK_CUR);
         // Now read in the entire header
         fread(headerBuffer, 1, headerSize, inputFile);
         // Now set dataHeader from headerBuffer
         // any extra information stored in the file
         // will be ignored
         memcpy((char*) dataHeader, headerBuffer, sizeof(tPBWaveformDataHeader));
         success = 1;
         // Just is case WaveformType has been enhanced
         if (dataHeader->BufferType > PB_DATA_LOGIC)
         {
            dataHeader->BufferType = PB_DATA_UNKNOWN;
         }
         // Done with headerBuffer
         free(headerBuffer);
      }
   }
   return success;
}
 
// Returns a buffer pointing the logic data read in if successful
// the client will be responisble for freeing the buffer
unsigned char* ReadLogicWaveform(FILE* inputFile,
                                 const tPBWaveformHeader* waveformHeader)
{
   tPBWaveformDataHeader dataHeader;
   unsigned char* pLogicData = NULL;
   if (ReadWaveformDataHeader(inputFile, &dataHeader) && waveformHeader)
   {
      // Make sure everything is the expected format
      int actualNumberOfPoints;
      actualNumberOfPoints = dataHeader.BufferSize / dataHeader.BytesPerPoint;
      if ((dataHeader.BytesPerPoint == 1) &&
          (dataHeader.BufferType == PB_DATA_LOGIC) &&
          (actualNumberOfPoints == waveformHeader->Points))
      {
         // Now let's read in the logic data
         pLogicData =(unsigned char*) malloc(dataHeader.BufferSize);
         if (pLogicData)
         {
            fread(pLogicData, 1, dataHeader.BufferSize, inputFile);
         }
      }
      if (pLogicData == NULL)
      {
         // ignore dataHeader.BufferSize because we either
         // did not allocate LogicData or we do not
         // recognize the data format
         fseek(inputFile, dataHeader.BufferSize, SEEK_CUR);
      }
   }
   return pLogicData;
}
 
// If bufferType != NULL, bufferType will be set.
// Returns a buffer with the analog data read in if successful
// the client will be responisble for freeing the buffer.
float* ReadAnalogWaveform(FILE* inputFile,
                          const tPBWaveformHeader* waveformHeader,
                          ePBDataType* bufferType)
{
   tPBWaveformDataHeader dataHeader;
   float* pWaveformData = NULL;
   if (ReadWaveformDataHeader(inputFile, &dataHeader) && waveformHeader)
   {
      // Make sure everything is the expected format
      int actualNumberOfPoints;
      int validDataType;
      actualNumberOfPoints = dataHeader.BufferSize / dataHeader.BytesPerPoint;
      validDataType = (dataHeader.BufferType == PB_DATA_NORMAL) ||
                      (dataHeader.BufferType == PB_DATA_MIN) ||
                      (dataHeader.BufferType == PB_DATA_MAX);
      if (bufferType != NULL)
      {
         *bufferType = dataHeader.BufferType;
      }
      if ((dataHeader.BytesPerPoint == 4) && validDataType &&
          (actualNumberOfPoints == waveformHeader->Points))
      {
         // Now let's read in the data
         pWaveformData =(float*) malloc(dataHeader.BufferSize);
         if (pWaveformData)
         {
            fread(pWaveformData, 1, dataHeader.BufferSize, inputFile);
         }
      }
      if (pWaveformData == NULL)
      {
         // ignore dataHeader.BufferSize because we either
         // did not allocate WaveformData or we do not
         // recognize the data format
         fseek(inputFile, dataHeader.BufferSize, SEEK_CUR);
         if (bufferType != NULL)
         {
            *bufferType = PB_DATA_UNKNOWN;
         }
      }
   }
   return pWaveformData;
}
 
// Returns a buffer with the histogram counts data read in if successful
// the client will be responisble for freeing the buffer
int* ReadHistogramWaveform(FILE* inputFile,
                           const tPBWaveformHeader* waveformHeader)
{
   int* pHistogramData = NULL;
   tPBWaveformDataHeader dataHeader;
   if (ReadWaveformDataHeader(inputFile, &dataHeader) && waveformHeader)
   {
      // Make sure everything is the expected format
      int actualNumberOfPoints;
      actualNumberOfPoints = dataHeader.BufferSize / dataHeader.BytesPerPoint;
      if ((dataHeader.BytesPerPoint == 4) &&
          (dataHeader.BufferType == PB_DATA_COUNTS) &&
          (actualNumberOfPoints == waveformHeader->Points))
      {
         // Now let's read in the histogram count data
         int* pHistogramData =(int*) malloc(dataHeader.BufferSize);
         if (pHistogramData)
         {
            fread(pHistogramData, 1, dataHeader.BufferSize, inputFile);
         }
      }
      if (pHistogramData == NULL)
      {
         // ignore dataHeader.BufferSize because we either
         // did not allocate pHistogramData or we do not
         // recognize the data format
         fseek(inputFile, dataHeader.BufferSize, SEEK_CUR);
      }
   }
   return pHistogramData;
}
 
// Moves the file forward past the current waveform data record
// including the data described.
// Returns 0 if not sucessful
int IgnoreWaveformData(FILE* inputFile)
{
   int success = 0;
   tPBWaveformDataHeader dataHeader;
   if (ReadWaveformDataHeader(inputFile, &dataHeader))
   {
      fseek(inputFile, dataHeader.BufferSize, SEEK_CUR);
      success = 1;
   }
   return success;
}
 
//*****************************************************************************
//
//  Description: The next set of functions demostrate how to use the above
//  functions of waveformHeader to generate a CSV file suitable for reading
//  into a spreadsheet application
//
double ComputeTimeFromIndex(int index, const tPBWaveformHeader* waveformHeader)
{
   return ((double) index * waveformHeader->XIncrement) + waveformHeader->XOrigin;
}
int OutputNormalData(FILE* inputFile,
                     const tPBWaveformHeader* waveformHeader,
                     FILE* outputFile)
{
   int    success = 0;
   float* waveformData = ReadAnalogWaveform(inputFile, waveformHeader, NULL);
   if (waveformData)
   {
      // Output Time and Voltage Data
      int i;
      for (i = 0; i < waveformHeader->Points; ++i)
      {
         double time = ComputeTimeFromIndex(i, waveformHeader);
         fprintf(outputFile, "%e, %f\n", time, waveformData[i]);
      }
      success = 1;
      // Client is responible for cleanup
      free(waveformData);
   }
   return success;
}
 
int OutputLogicData(FILE* inputFile,
                    const tPBWaveformHeader* waveformHeader,
                    FILE* outputFile)
{
   int success = 0;
   if (waveformHeader->NWaveformBuffers == 2)
   {
      // Two Pods stored
      unsigned char* podData1 = ReadLogicWaveform(inputFile,
                                                 waveformHeader);
      unsigned char* podData2 = ReadLogicWaveform(inputFile,
                                                 waveformHeader);
      if (podData1 && podData2)
      {
         // Output Time and Logic Data
         int i;
         for (i = 0; i < waveformHeader->Points; ++i)
         {
            double time = ComputeTimeFromIndex(i, waveformHeader);
            fprintf(outputFile, "%e, %x%x\n", time, podData2[i], podData1[i]);
         }
         success = 1;
         // Client is responsible for freeing memory
         free(podData1);
         free(podData2);
      }
   }
   else
   {
      // Only a single pod
      unsigned char* podData = ReadLogicWaveform(inputFile,
                                                 waveformHeader);
      if (podData)
      {
         // Output Time and Logic Data
         int i;
         for (i = 0; i < waveformHeader->Points; ++i)
         {
            double time = ComputeTimeFromIndex(i, waveformHeader);
            fprintf(outputFile, "%e, %x\n", time, podData[i]);
         }
         success = 1;
         // Client is responsible for freeing memory
         free(podData);
      }
   }
   return success;
}
 
int OutputHistogramData(FILE* inputFile,
                        const tPBWaveformHeader* waveformHeader,
                        FILE* outputFile)
{
   int    success = 0;
   int* histogramData = ReadHistogramWaveform(inputFile, waveformHeader);
   if (histogramData)
   {
      // Output Time and Count Data
      int i;
      for (i = 0; i < waveformHeader->Points; ++i)
      {
         double time = ComputeTimeFromIndex(i, waveformHeader);
         fprintf(outputFile, "%e, %i\n", time, histogramData[i]);
      }
      success = 1;
      // Client is responible for cleanup
      free(histogramData);
   }
   return success;
}
 
int OutputPeakDetectData(FILE* inputFile,
                         const tPBWaveformHeader* waveformHeader,
                         FILE* outputFile)
{
   int    success = 0;
   float* minData;
   float* maxData;
   float* tempData;
   ePBDataType bufferType;
   minData = maxData = NULL;
   tempData = ReadAnalogWaveform(inputFile, waveformHeader, &bufferType);
   if (bufferType == PB_DATA_MIN)
   {
      minData = tempData;
      maxData = ReadAnalogWaveform(inputFile, waveformHeader, &bufferType);
   }
   else if (bufferType == PB_DATA_MAX)
   {
      maxData = tempData;
      minData = ReadAnalogWaveform(inputFile, waveformHeader, &bufferType);
   }
   if (maxData && minData)
   {
      // Output Time and Voltage Data
      int i;
      for (i = 0; i < waveformHeader->Points; ++i)
      {
         double time = ComputeTimeFromIndex(i, waveformHeader);
         fprintf(outputFile, "%e, %f, %f\n", time, minData[i], maxData[i]);
      }
      success = 1;
   }
   // Client is responible for cleanup
   free(minData);
   free(maxData);
   return success;
}
 
void OutputSummary(const tPBWaveformHeader* waveformHeader, FILE* outputFile)
{
   static const char* waveformTable[] =
   {
      "PB_UNKNOWN",
      "PB_NORMAL",
      "PB_PEAK_DETECT",
      "PB_AVERAGE",
      "PB_HORZ_HISTOGRAM",
      "PB_VERT_HISTOGRAM",
      "PB_LOGIC"
   };
   fprintf(outputFile, "%s, %s, ",
           waveformHeader->WaveformLabel,
           waveformTable[ waveformHeader->WaveformType]);
   // Segmented Memory waveforms will have a SegmentIndex > 1
   if (waveformHeader->SegmentIndex > 0)
   {
      fprintf(outputFile, "%d, ", waveformHeader->SegmentIndex);
   }
   fprintf(outputFile, "%d, %s, %s, %s\n",
           waveformHeader->Points,
           waveformHeader->Frame,
           waveformHeader->Date,
           waveformHeader->Time);
}
 
int SummarizeWaveform(FILE* inputFile, FILE* outputFile)
{
   int success = 0;
   int w;
   tPBWaveformHeader waveformHeader;
   if (ReadWaveformHeader(inputFile, &waveformHeader))
   {
      // write out basic summary
      OutputSummary(&waveformHeader, outputFile);
      // ignore the waveform data
      for (w = 0; w < waveformHeader.NWaveformBuffers; ++w)
      {
         success = IgnoreWaveformData(inputFile);
      }
   }
   return success;
}
 
int OutputWaveform(FILE* inputFile, FILE* outputFile)
{
   int success = 0;
   int w;
   tPBWaveformHeader waveformHeader;
   if (ReadWaveformHeader(inputFile, &waveformHeader))
   {
      // write out basic summary
      //OutputSummary(&waveformHeader, outputFile);
      // write out waveform data
      switch(waveformHeader.WaveformType)
      {
         case PB_NORMAL:
         case PB_AVERAGE:
            success = OutputNormalData(inputFile, &waveformHeader, outputFile);
            break;
         case PB_PEAK_DETECT:
            success = OutputPeakDetectData(inputFile, &waveformHeader, outputFile);
            break;
         case PB_HORZ_HISTOGRAM:
         case PB_VERT_HISTOGRAM:
            success = OutputHistogramData(inputFile, &waveformHeader, outputFile);
            break;
         case PB_LOGIC:
            success = OutputLogicData(inputFile, &waveformHeader, outputFile);
            break;
         default:
         case PB_UNKNOWN:
            for(w = 0; w < waveformHeader.NWaveformBuffers; ++w)
            {
               IgnoreWaveformData(inputFile);
            }
            break;
      }
   }
   return success;
}
 
int main(int argc, char** argv)
{
   FILE* inputFile;
   if (argc < 2)
   {
      printf("binToAscii <input file> <output file 1> ... <output file n>\n");
      return 0;
   }
   inputFile = fopen(argv[1], "rb");
   if (inputFile)
   {
      tPBFileHeader fileHeader;
      fread((char*) &fileHeader, 1, sizeof(fileHeader), inputFile);
      // verify cookie
      if (fileHeader.Cookie[0] == PB_COOKIE[0] &&
         fileHeader.Cookie[1] == PB_COOKIE[1])
      {
         int w;
         if ((argc - 2) < fileHeader.NumberOfWaveforms)
         {
            // Not enough output files were provided
            // Use stdout to summarize input file
            printf("Infiniium Public Waveform File version %c.%c\n",
                   fileHeader.Version[0],
                   fileHeader.Version[1]);
            for (w = 0; w < fileHeader.NumberOfWaveforms; ++w)
            {
               SummarizeWaveform(inputFile, stdout);
            }
         }
         else
         {
            for (w = 0; w < fileHeader.NumberOfWaveforms; ++w)
            {
               FILE* outputFile = fopen(argv[w + 2], "w");
               if (outputFile)
               {
                  OutputWaveform(inputFile, outputFile);
                  fclose(outputFile);
               }
               else
               {
                  printf("Unable to open %s\n", argv[w + 2]);
               }
            }
         }
      }
      else
      {
         printf("Invalid Infiniium Public Waveform File\n");
      }
      fclose(inputFile);
   }
   else
   {
      printf("Unable to open %s\n", argv[1]);
   }
   return 0;
}
