#ifndef VIDEOFORMAT_H
#define VIDEOFORMAT_H
#include <QString>
#include <iostream>
#include <fstream>
#include "lib/handler/loghandler.h"


enum VideoFormatResultCode
{
   E_Error   = -1,
   E_OK      = 0,
   E_No3D    = 2,
   E_Found3D = 3,

};

class VideoFormat
{
    typedef unsigned char uint8_t;

    enum NALType
    {
       NALType_Unknown             =  0,
       NALType_Slice               =  1,
       NALType_Slice_DPA           =  2,
       NALType_Slice_DPB           =  3,
       NALType_Slice_DPC           =  4,
       NALType_Slice_IDR           =  5,
       NALType_SEI                 =  6,
       NALType_SPS                 =  7,
       NALType_PPS                 =  8,
       NALType_AU_Delimiter        =  9,
       NALType_SequenceEnd         = 10,
       NALType_StreamEnd           = 11,
       NALType_FillerData          = 12,
       NALType_CodedSliceExtension = 20,

       NALType_MAX                 = 0x1f
    };

    enum SEIType
    {
       SEIType_FramePackingArrangement = 0x2D
    };


    enum StartCodeState
    {
       StartCodeState_none,
       StartCodeState_0,
       StartCodeState_0_0,
       StartCodeState_0_0_1
    };


public:
    int is3D(char* filepath)
    {
        std::ifstream inputFile(filepath, std::ios::binary);

        int nResult = E_OK;
        if( inputFile.is_open() )
        {
           StartCodeState eStartCodeState = StartCodeState_none;

           while( (E_OK == nResult) && ( ! inputFile.eof() ) )
           {
              uint8_t byte = inputFile.get();

              switch(eStartCodeState)
              {
                 case  StartCodeState_none :
                    eStartCodeState = (byte == 0) ? StartCodeState_0 : StartCodeState_none;
                    break;

                 case  StartCodeState_0 :
                    eStartCodeState = (byte == 0) ? StartCodeState_0_0 : StartCodeState_none;
                    break;

                 case  StartCodeState_0_0 :
                    switch(byte)
                    {
                       case 0  : eStartCodeState = StartCodeState_0_0;   break;
                       case 1  : eStartCodeState = StartCodeState_0_0_1; break;
                       default : eStartCodeState = StartCodeState_none;

                    }

                 default:
                    ;
              }

              if(  eStartCodeState == StartCodeState_0_0_1 )
              {
                 uint8_t cNALType  = inputFile.get();
                         cNALType &= NALType_MAX;

                 switch(cNALType)
                 {
                    case NALType_CodedSliceExtension :
                       nResult = E_Found3D;
                       break;

                    case NALType_SEI :
                    {
                       uint8_t cSEIType  = inputFile.get();
                       if( cSEIType == SEIType_FramePackingArrangement )
                       {
                          nResult = E_Found3D;
                       }
                       break;
                    }

                    default:
                       ;
                 }

                 eStartCodeState = StartCodeState_none;
              }
           }
        }
        else
        {
            LogHandler::Warn("Error opening: "+QString(filepath));
            nResult = E_Error;
        }

       return nResult;
    }
};


#endif // VIDEOFORMAT_H
