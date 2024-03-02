#include "bmp_image.h"
#include "pack_defines.h"


#include <array>
#include <fstream>
#include <string_view>
#include <iostream>

using namespace std;

namespace img_lib {
// функция вычисления отступа по ширине
static int GetBMPpadding(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    char bfType[2] = {};
    uint32_t bfSize = {};
    uint32_t bfReserved = 0;
    uint32_t bfOffBits = 54; 
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    // поля заголовка Bitmap Info Header
    uint32_t biSize = 40; 
    int32_t biWidth = {};
    int32_t biHeight = {};
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 24;
    uint32_t biCompression = 0;
    uint32_t biSizeImage = {};
    int32_t biXPelsPerMeter = 11811;
    int32_t biYPelsPerMeter = 11811;
    int32_t biClrUsed = 0;
    int32_t biClrImportant = 0x1000000;
}
PACKED_STRUCT_END


bool SaveBMP(const Path& file, const Image& image) {

    const int padding = GetBMPpadding(image.GetWidth());

    BitmapFileHeader file_header;
    file_header.bfType[0] = {'B'};
    file_header.bfType[1] = {'M'};
    file_header.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + padding * image.GetHeight();

    BitmapInfoHeader info_header;
    info_header.biWidth = image.GetWidth();
    info_header.biHeight = image.GetHeight();
    info_header.biSizeImage = GetBMPpadding(image.GetWidth()) * image.GetHeight();
    
    ofstream out(file, ios::binary);
    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
    
    vector<char> buff(padding);
    
    for (int y = image.GetHeight() - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < image.GetWidth(); ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(reinterpret_cast<const char*>(buff.data()), buff.size());
    }

    return out.good();
}


Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    
    if (!ifs.is_open()) {
        Image null(0, 0, Color::Black());
        return null;
    }
    
   BitmapFileHeader file_header; 
   BitmapInfoHeader info_header;

    ifs.read(reinterpret_cast<char*>(&file_header), 14);
    ifs.read(reinterpret_cast<char*>(&info_header), 40);

    if (file_header.bfType[0] != 'B' || file_header.bfType[1] != 'M') {
         Image null(0, 0, Color::Black());
         return null;
    }

    const int imafge_width = info_header.biWidth;
    const int image_hight = info_header.biHeight;
    const int padding = GetBMPpadding(imafge_width);

    Image result(padding / 3, image_hight, Color::Black());
    std::vector<char> buff(padding);

    for (int y = result.GetHeight() - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), padding);

        for (int x = 0; x < imafge_width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }
    return result; 
}

}  // namespace img_lib
