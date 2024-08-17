#pragma once

#ifdef COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "./win32_base.h"
#include "../core/image.h"


u8* loadBitmap(char *filename, ImageInfo &info) {
    void *file = os::openFileForReading(filename);
    if (!file) return nullptr;

    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    os::readFromFile(&file_header, sizeof(BITMAPFILEHEADER), file);
    os::readFromFile(&info_header, sizeof(BITMAPINFOHEADER), file);
    if (file_header.bfType != 0x4D42) {
        os::closeFile(file);
        return nullptr;
    }

    bool flipped = info_header.biHeight > 0;
    if (info_header.biBitCount == 32) info.flags.alpha = true;
    info.updateDimensions(info_header.biWidth, flipped ? info_header.biHeight : -info_header.biHeight);

    u32 component_count = info.flags.alpha ? 4 : 3;
    u32 size_in_bytes = component_count * info.size;
    u8 *components = new u8[size_in_bytes];
    u8 *scratch_components = new u8[size_in_bytes];

    SetFilePointer(file, (LONG)file_header.bfOffBits, nullptr, FILE_BEGIN);
    os::readFromFile(components, size_in_bytes, file);
    os::closeFile(file);

    if (flipped != info.flags.flip) {
        flipImage(components, info, scratch_components);
        for (u32 i = 0; i < size_in_bytes; i++) components[i] = scratch_components[i];
    }

    if (info.flags.tile) {
        info.updateTileDimensions(8, 4);
        tileImage(components, info, scratch_components);
        for (u32 i = 0; i < size_in_bytes; i++) components[i] = scratch_components[i];
    }

    delete[] scratch_components;

    return components;
}


//PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
//{
//    BITMAP bmp;
//    PBITMAPINFO pbmi;
//    WORD    cClrBits;
//
//    // Retrieve the bitmap color format, width, and height.
//    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
//        errhandler("GetObject", hwnd);
//
//    // Convert the color format to a count of bits.
//    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
//    if (cClrBits <= 24)
//        cClrBits = 24;
//    else
//        cClrBits = 32;
//
//    // Allocate memory for the BITMAPINFO structure. (This structure contains a BITMAPINFOHEADER structure and an array of RGBQUAD data structures.)
//    pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
//
//    // Initialize the fields in the BITMAPINFO structure.
//    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
//    pbmi->bmiHeader.biWidth = bmp.bmWidth;
//    pbmi->bmiHeader.biHeight = bmp.bmHeight;
//    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
//    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
//    pbmi->bmiHeader.biCompression = BI_RGB;
//
//    // Compute the number of bytes in the array of color indices and store the result in biSizeImage.
//    // The width must be DWORD aligned
//    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8 * pbmi->bmiHeader.biHeight;
//    pbmi->bmiHeader.biClrImportant = 0; // Set biClrImportant to 0, indicating that all of the device colors are important.
//    return pbmi;
//}
//
//void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
//                   HBITMAP hBMP, HDC hDC)
//{
//    HANDLE hf;                 // file handle
//    BITMAPFILEHEADER hdr;       // bitmap file-header
//    PBITMAPINFOHEADER pbih;     // bitmap info-header
//    LPBYTE lpBits;              // memory pointer
//    DWORD dwTotal;              // total count of bytes
//    DWORD cb;                   // incremental count of bytes
//    BYTE *hp;                   // byte pointer
//    DWORD dwTmp;
//
//    pbih = (PBITMAPINFOHEADER) pbi;
//    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);
//
//    if (!lpBits)
//        errhandler("GlobalAlloc", hwnd);
//
//    // Retrieve the color table (RGBQUAD array) and the bits
//    // (array of palette indices) from the DIB.
//    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi,
//                   DIB_RGB_COLORS))
//    {
//        errhandler("GetDIBits", hwnd);
//    }
//
//    // Create the .BMP file.
//    hf = CreateFile(pszFile,
//                    GENERIC_READ | GENERIC_WRITE,
//                    (DWORD) 0,
//                    NULL,
//                    CREATE_ALWAYS,
//                    FILE_ATTRIBUTE_NORMAL,
//                    (HANDLE) NULL);
//    if (hf == INVALID_HANDLE_VALUE)
//        errhandler("CreateFile", hwnd);
//    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
//    // Compute the size of the entire file.
//    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +
//                          pbih->biSize + pbih->biClrUsed
//                                         * sizeof(RGBQUAD) + pbih->biSizeImage);
//    hdr.bfReserved1 = 0;
//    hdr.bfReserved2 = 0;
//
//    // Compute the offset to the array of color indices.
//    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
//                    pbih->biSize + pbih->biClrUsed
//                                   * sizeof (RGBQUAD);
//
//    // Copy the BITMAPFILEHEADER into the .BMP file.
//    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER),
//                   (LPDWORD) &dwTmp,  NULL))
//    {
//        errhandler("WriteFile", hwnd);
//    }
//
//    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
//    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER)
//                                      + pbih->biClrUsed * sizeof (RGBQUAD),
//                   (LPDWORD) &dwTmp, ( NULL)))
//        errhandler("WriteFile", hwnd);
//
//    // Copy the array of color indices into the .BMP file.
//    dwTotal = cb = pbih->biSizeImage;
//    hp = lpBits;
//    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL))
//        errhandler("WriteFile", hwnd);
////
////    // Close the .BMP file.
////    if (!CloseHandle(hf))
////        errhandler("CloseHandle", hwnd);
////
////    // Free memory.
////    GlobalFree((HGLOBAL)lpBits);
//}