/*
% Copyright (C) 2003 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
% Copyright 1991-1999 E. I. du Pont de Nemours and Company
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            RRRR   L       AAA                               %
%                            R   R  L      A   A                              %
%                            RRRR   L      AAAAA                              %
%                            R R    L      A   A                              %
%                            R  R   LLLLL  A   A                              %
%                                                                             %
%                                                                             %
%                     Read Alias/Wavefront Image Format.                      %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/cache.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d R L A I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadRLAImage reads a run-length encoded Wavefront RLA image file
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  Note:  This module was contributed by Lester Vecsey (master@internexus.net).
%
%  The format of the ReadRLAImage method is:
%
%      Image *ReadRLAImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadRLAImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadRLAImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  typedef struct _WindowFrame
  {
    short
      left,
      right,
      bottom,
      top;
  } WindowFrame;

  typedef struct _RLAInfo
  {
    WindowFrame
      window,
      active_window;

    magick_uint16_t
      frame,
      storage_type,
      number_channels,
      number_matte_channels,
      number_auxiliary_channels,
      revision; /* aux_mask in RLB */

    char
      gamma[16],
      red_primary[24],
      green_primary[24],
      blue_primary[24],
      white_point[24];

    magick_uint32_t
      job_number;

    char
      name[128],
      description[128],
      program[64],
      machine[32],
      user[32],
      date[20],
      aspect[24],
      aspect_ratio[8],
      chan[32];

    magick_uint16_t
      field;

    /* RLB varies after this point */

  } RLAInfo;

  typedef struct _RLA3ExtraInfo
  {
    char
      time[12],
      filter[32];

    magick_uint16_t
      bits_per_channel,
      matte_type,
      matte_bits,
      auxiliary_type,
      auxiliary_bits;

    char
      auxiliary[32],
      space[36];

    magick_uint32_t
      next;
  } RLA3ExtraInfo;

  typedef struct _RLBExtraInfo
  {
    magick_uint16_t
      filter_type;

    magick_uint32_t
      magic_number,
      lut_size,
      user_space_size,
      wf_space_size;

    magick_uint16_t
      lut_type,
      mix_type,
      encode_type,
      padding;

    char
      space[100];
  } RLBExtraInfo;

  Image
    *image;

  int
    channel,
    length,
    number_channels,
    runlength;

  long
    y;

  magick_uint32_t
    *scanlines;

  register long
    i,
    x;

  register PixelPacket
    *q;

  RLAInfo
    rla_info;

  RLA3ExtraInfo
    rla3_extra_info;

  RLBExtraInfo
    rlb_extra_info;

  MagickBool
    is_rla3;

  int
    byte;

  MagickPassFail
    status;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AllocateImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFail)
    ThrowReaderException(FileOpenError,UnableToOpenFile,image);
  is_rla3=MagickFalse;
  memset(&rla_info,0,sizeof(rla_info));
  memset(&rla3_extra_info,0,sizeof(rla3_extra_info));
  memset(&rlb_extra_info,0,sizeof(rlb_extra_info));
  rla_info.window.left=ReadBlobMSBShort(image);
  rla_info.window.right=ReadBlobMSBShort(image);
  rla_info.window.bottom=ReadBlobMSBShort(image);
  rla_info.window.top=ReadBlobMSBShort(image);
  rla_info.active_window.left=ReadBlobMSBShort(image);
  rla_info.active_window.right=ReadBlobMSBShort(image);
  rla_info.active_window.bottom=ReadBlobMSBShort(image);
  rla_info.active_window.top=ReadBlobMSBShort(image);
  rla_info.frame=ReadBlobMSBShort(image);
  rla_info.storage_type=ReadBlobMSBShort(image);
  rla_info.number_channels=ReadBlobMSBShort(image);
  rla_info.number_matte_channels=ReadBlobMSBShort(image);
  if (rla_info.number_channels == 0)
    rla_info.number_channels=3;
  rla_info.number_auxiliary_channels=ReadBlobMSBShort(image);
  rla_info.revision=ReadBlobMSBShort(image);
  if (rla_info.revision == 0xFFFE)
    is_rla3=MagickTrue;
  (void) ReadBlob(image,16,(char *) rla_info.gamma);
  (void) ReadBlob(image,24,(char *) rla_info.red_primary);
  (void) ReadBlob(image,24,(char *) rla_info.green_primary);
  (void) ReadBlob(image,24,(char *) rla_info.blue_primary);
  (void) ReadBlob(image,24,(char *) rla_info.white_point);
  rla_info.job_number=(long) ReadBlobMSBLong(image);
  (void) ReadBlob(image,128,(char *) rla_info.name);
  (void) ReadBlob(image,128,(char *) rla_info.description);
  (void) ReadBlob(image,64,(char *) rla_info.program);
  (void) ReadBlob(image,32,(char *) rla_info.machine);
  (void) ReadBlob(image,32,(char *) rla_info.user);
  (void) ReadBlob(image,20,(char *) rla_info.date);
  (void) ReadBlob(image,24,(char *) rla_info.aspect);
  (void) ReadBlob(image,8,(char *) rla_info.aspect_ratio);
  (void) ReadBlob(image,32,(char *) rla_info.chan);
  rla_info.field=ReadBlobMSBShort(image);
  if (is_rla3)
    {
      (void) ReadBlob(image,12,(char *) rla3_extra_info.time);
      (void) ReadBlob(image,32,(char *) rla3_extra_info.filter);
      rla3_extra_info.bits_per_channel=ReadBlobMSBShort(image);
      rla3_extra_info.matte_type=ReadBlobMSBShort(image);
      rla3_extra_info.matte_bits=ReadBlobMSBShort(image);
      rla3_extra_info.auxiliary_type=ReadBlobMSBShort(image);
      rla3_extra_info.auxiliary_bits=ReadBlobMSBShort(image);
      (void) ReadBlob(image,32,(char *) rla3_extra_info.auxiliary);
      (void) ReadBlob(image,36,(char *) rla3_extra_info.space);
      rla3_extra_info.next=(long) ReadBlobMSBLong(image);
    }
  else
    {
      rlb_extra_info.filter_type=ReadBlobMSBShort(image);
      rlb_extra_info.magic_number=ReadBlobMSBLong(image);
      rlb_extra_info.lut_size=ReadBlobMSBLong(image);
      rlb_extra_info.user_space_size=ReadBlobMSBLong(image);
      rlb_extra_info.wf_space_size=ReadBlobMSBLong(image);
      rlb_extra_info.lut_type=ReadBlobMSBShort(image);
      rlb_extra_info.mix_type=ReadBlobMSBShort(image);
      rlb_extra_info.encode_type=ReadBlobMSBShort(image);
      rlb_extra_info.padding=ReadBlobMSBShort(image);
      (void) ReadBlob(image,100,(char *) rlb_extra_info.space);
    }
  if (EOFBlob(image))
    ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,image);

  /*
    Verify revision.
  */

/*   if (rla3_extra_info.revision != 0xFFFE) */
/*     ThrowReaderException(CorruptImageError,ImproperImageHeader,image); */

  /*
    Verify dimensions.
  */
  if ((((long) rla_info.active_window.right - rla_info.active_window.left) < 0) ||
      (((long) rla_info.active_window.top-rla_info.active_window.bottom) < 0))
    ThrowReaderException(CorruptImageError,ImproperImageHeader,image);

  if ((rla_info.storage_type != 0) || (rla_info.storage_type > 3))
    ThrowReaderException(CorruptImageError,ImproperImageHeader,image);

  if (rla_info.storage_type != 0)
    ThrowReaderException(CoderError,DataStorageTypeIsNotSupported,image);

  if (LocaleNCompare(rla_info.chan,"rgb",3) != 0)
    ThrowReaderException(CoderError,ColorTypeNotSupported,image);
                          
  /*
    Initialize image structure.
  */
  image->matte=(rla_info.number_matte_channels != 0 ? MagickTrue: MagickFalse);
  image->columns=rla_info.active_window.right-rla_info.active_window.left+1;
  image->rows=rla_info.active_window.top-rla_info.active_window.bottom+1;
  if (image_info->ping)
    {
      CloseBlob(image);
      return(image);
    }
  number_channels=rla_info.number_channels+rla_info.number_matte_channels;
  scanlines=MagickAllocateMemoryElements(magick_uint32_t *,image->rows,sizeof(magick_uint32_t));
  if (scanlines == (magick_uint32_t *) NULL)
    ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
  if (*rla_info.description != '\0')
    (void) SetImageAttribute(image,"comment",rla_info.description);
  /*
    Read offsets to each scanline data.
  */
  for (i=0; i < (long) image->rows; i++)
      scanlines[i]=(magick_uint32_t) ReadBlobMSBLong(image);
  if (EOFBlob(image))
    ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,image);
  /*
    Read image data.
  */
  x=0;
  for (y=0; y < (long) image->rows; y++)
  {
    if (SeekBlob(image,scanlines[image->rows-y-1],SEEK_SET) == -1)
      {
        status=MagickFail;
        break;
      }
    for (channel=0; channel < number_channels; channel++)
    {
      length=ReadBlobMSBShort(image);
      while (length > 0)
      {
        if ((byte=ReadBlobByte(image)) == EOF)
          {
            status=MagickFail;
            break;
          }
        runlength=byte;
        if (byte > 127)
          runlength=byte-256;
        length--;
        if (length == 0)
          break;
        if (runlength < 0)
          {
            while (runlength < 0)
            {
              q=GetImagePixels(image,(long) (x % image->columns),
                               (long) (y % image->columns),1,1);
              if (q == (PixelPacket *) NULL)
                {
                  status=MagickFail;
                  break;
                }
              if ((byte=ReadBlobByte(image)) == EOF)
                {
                  status=MagickFail;
                  break;
                }
              length--;
              switch (channel)
              {
                case 0:
                {
                  q->red=ScaleCharToQuantum(byte);
                  break;
                }
                case 1:
                {
                  q->green=ScaleCharToQuantum(byte);
                  break;
                }
                case 2:
                {
                  q->blue=ScaleCharToQuantum(byte);
                  break;
                }
                case 3:
                {
                  q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(byte));
                  break;
                }
                default:
                {
                  /* Depth channel ? */
                  break;
                }
              }
              if (!SyncImagePixels(image))
                {
                  status=MagickFail;
                  break;
                }
              x++;
              runlength++;
            }
            continue;
          }
        if ((byte=ReadBlobByte(image)) == EOF)
          {
            status=MagickFail;
            break;
          }
        length--;
        runlength++;
        do
        {
          q=GetImagePixels(image,(long) (x % image->columns),
            (long) (y % image->columns),1,1);
          if (q == (PixelPacket *) NULL)
            {
              status=MagickFail;
              break;
            }
          switch (channel)
          {
            case 0:
            {
              q->red=ScaleCharToQuantum(byte);
              break;
            }
            case 1:
            {
              q->green=ScaleCharToQuantum(byte);
              break;
            }
            case 2:
            {
              q->blue=ScaleCharToQuantum(byte);
              break;
            }
            case 3:
            {
              q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(byte));
              break;
            }
          default:
            {
              /* Depth channel ? */
              break;
            }
          }
          if (!SyncImagePixels(image))
            {
              status=MagickFail;
              break;
            }
          x++;
          runlength--;
        }
        while (runlength > 0);

        if (MagickFail == status)
          break;
      }
      if (MagickFail == status)
        break;
    }
    if (QuantumTick(y,image->rows))
      if (!MagickMonitor(LoadImageText,y,image->rows,exception))
        {
          status=MagickFail;
          break;
        }
    if (MagickFail == status)
      break;
  }
  if (EOFBlob(image))
    ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,image);
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r R L A I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterRLAImage adds attributes for the RLA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterRLAImage method is:
%
%      RegisterRLAImage(void)
%
*/
ModuleExport void RegisterRLAImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("RLA");
  entry->decoder=(DecoderHandler) ReadRLAImage;
  entry->adjoin=False;
  entry->description=AcquireString("Alias/Wavefront image");
  entry->module=AcquireString("RLA");
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r R L A I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterRLAImage removes format registrations made by the
%  RLA module from the list of supported formats.
%
%  The format of the UnregisterRLAImage method is:
%
%      UnregisterRLAImage(void)
%
*/
ModuleExport void UnregisterRLAImage(void)
{
  (void) UnregisterMagickInfo("RLA");
}
