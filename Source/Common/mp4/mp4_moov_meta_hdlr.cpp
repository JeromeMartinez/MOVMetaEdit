// BWF MetaEdit Riff - RIFF stuff for BWF MetaEdit
//
// This code was created in 2010 for the Library of Congress and the
// other federal government agencies participating in the Federal Agencies
// Digitization Guidelines Initiative and it is in the public domain.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "Common/mp4/mp4_.h"
//---------------------------------------------------------------------------

//***************************************************************************
// WAVE
//***************************************************************************

//---------------------------------------------------------------------------
void mp4_moov_meta_hdlr::Read_Internal ()
{
    //Reading
    Read_Internal_ReadAllInBuffer();

    int32u Handler_type;
    int8u Version;
    Get_B1( Version);
    if (Version)
        throw exception_valid("moov meta version unsupported");
    Skip_XX(3+4); //Flags+Predefined
    Get_B4( Handler_type);
    if (Handler_type!=0x6D647461) //mdta
        throw exception_valid("moov meta Handler type unsupported");
}

//***************************************************************************
// Modify
//***************************************************************************

//---------------------------------------------------------------------------
void mp4_moov_meta_hdlr::Modify_Internal()
{
    if (Chunk.Content.Size)
    {
        return; //Nothing to do
    }

    //Creating buffer
    Chunk.Content.Buffer_Offset = 0;
    Chunk.Content.Size = 24;
    Chunk.Content.Buffer = new int8u[Chunk.Content.Size];

    Put_B4(0x00000000);
    Put_B4(0x00000000);
    Put_B4(0x6D647461);
    Put_B4(0x00000000);
    Put_B4(0x00000000);
    Put_B4(0x00000000);

    Chunk.Content.IsModified = true;
    Chunk.Content.Size_IsModified = true;
}

//***************************************************************************
// Write
//***************************************************************************

//---------------------------------------------------------------------------
void mp4_moov_meta_hdlr::Write_Internal()
{
    mp4_Base::Write_Internal(Chunk.Content.Buffer, (size_t)Chunk.Content.Size);
}
