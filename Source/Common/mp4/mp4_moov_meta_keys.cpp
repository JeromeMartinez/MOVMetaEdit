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
void mp4_moov_meta_keys::Read_Internal ()
{
    //Integrity
    if (Global->moov_meta_keys_AlreadyPresent)
        throw exception_valid("2 moov meta keys chunks");

    //Reading
    Read_Internal_ReadAllInBuffer();

    int32u Entry_count;
    int8u Version;
    Get_B1(Version);
    if (Version)
        throw exception_valid("moov meta keys version unsupported");
    Skip_XX(3); //Flags
    Get_B4(Entry_count);
    while (Chunk.Content.Buffer_Offset<Chunk.Content.Size)
    {
        int32u Key_size, Key_namespace;
        Get_B4(Key_size);
        if (Key_size<8)
            throw exception_valid("moov meta keys Key_size invalid");
        string Key_value;
        Get_B4(Key_namespace);
        Get_String(Key_size-8, Key_value);
        if (Key_namespace!=0x6D647461 && (Key_value=="com.universaladid.idregistry" || Key_value=="com.universaladid.idvalue")) //mdta
            throw exception_valid("Ad-ID fields not mdta unsupported");
        //Global->moov_meta_keys.push_back(Key_value);
        Global->moov_meta_keys_AlreadyPresent++;

        //TEMP
        if ((Key_value == "com.universaladid.idregistry" || Key_value == "com.universaladid.idvalue")) //mdta
            throw exception_valid("Ad-ID fields already present");
    }

    if (Entry_count!=Global->moov_meta_keys_AlreadyPresent)
        throw exception_valid("moov meta keys incoherant");
}

//***************************************************************************
// Modify
//***************************************************************************

//---------------------------------------------------------------------------
void mp4_moov_meta_keys::Modify_Internal()
{
    if (!Global->moov_meta_keys_AlreadyPresent && Global->moov_meta_keys_NewKeys.empty())
    {
        Chunk.Content.IsRemovable = true;
        return;
    }

    if (Global->moov_meta_keys_NewKeys.empty())
        return; //No change

    //Compute of content size to add
    size_t Size_ToAdd=0;
    for (size_t i=0; i<Global->moov_meta_keys_NewKeys.size(); i++)
        Size_ToAdd+=8+Global->moov_meta_keys_NewKeys[i].size();

    //Changing count
    if (Chunk.Content.Size)
    {
        //Version+Flags+Count
        Chunk.Content.Buffer_Offset=4; //Skipping Version+Flags
        Put_B4(Global->moov_meta_keys_AlreadyPresent+Global->moov_meta_keys_NewKeys.size()); //Count

        //Creating buffer
        Chunk.Content.Buffer_Offset=Chunk.Content.Size;
        int8u* t=new int8u[Chunk.Content.Size+Size_ToAdd];
        memcpy(t, Chunk.Content.Buffer, Chunk.Content.Size);
        delete[] Chunk.Content.Buffer;
        Chunk.Content.Buffer=t;
        Chunk.Content.Size+=Size_ToAdd;
    }
    else
    {
        //Creating buffer
        Chunk.Content.Buffer_Offset=0;
        Chunk.Content.Size=8+Size_ToAdd;
        Chunk.Content.Buffer=new int8u[Chunk.Content.Size];

        //Version+Flags+Count
        Put_B4(0x00000000); //Version+Flags
        Put_B4(Global->moov_meta_keys_NewKeys.size()); //Count
    }

    //New keys
    for (size_t i=0; i<Global->moov_meta_keys_NewKeys.size(); i++)
    {
        Put_B4(8+Global->moov_meta_keys_NewKeys[i].size());
        Put_B4(0x6D647461); //mdta
        Put_String(Global->moov_meta_keys_NewKeys[i].size(), Global->moov_meta_keys_NewKeys[i]);
    }
    Global->moov_meta_keys_AlreadyPresent+=Global->moov_meta_keys_NewKeys.size();
    Global->moov_meta_keys_NewKeys.clear();

    Chunk.Content.IsModified = true;
    Chunk.Content.Size_IsModified = true;
}

//***************************************************************************
// Write
//***************************************************************************

//---------------------------------------------------------------------------
void mp4_moov_meta_keys::Write_Internal()
{
    mp4_Base::Write_Internal(Chunk.Content.Buffer, (size_t)Chunk.Content.Size);
}
