#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <string>
#include "image_ref.h"

int save_image( void * data, const ImageRef & size, const int depth, const std::wstring & filename ); 

#endif IMAGE_IO_H
