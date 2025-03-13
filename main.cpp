#include <iostream>
#include <vector>
#include <fstream>
using namespace std;
struct Pixel{
    
    int red;
    int green;
    int blue;
};

int get_int(fstream& stream, int offset, int bytes){
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++){   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

vector<vector<Pixel>> read_image(string filename){
    
    fstream stream;
    string fullPath = "../" + filename;
    stream.open(fullPath, ios::in | ios::binary); // ios::in - režim čítania(input mode)

    if (!stream.is_open()) {
        cout << "Error: neda sa otvorit'" << filename << "'" << endl;
        return {};
    }

    int file_size = get_int(stream, 2, 4);
    //cout << "file_size: " << file_size << endl;
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

   int bytes_per_row = width * (bits_per_pixel / 8);  // bez paddingu
    int padding = 0;
    if (bytes_per_row % 4 != 0){
        padding = 4 - bytes_per_row % 4;
    }

    if (file_size != start + (bytes_per_row + padding) * height){
        return {};
    }

    vector<vector<Pixel> > image(height, vector<Pixel> (width));
    int pos = start;
    for (int i = height - 1; i >= 0; i--){
      
        for (int j = 0; j < width; j++){
            stream.seekg(pos);

            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            pos = pos + (bits_per_pixel / 8);
        }
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

  	stream.close();
    return image;
}

void set_bytes(unsigned char arr[], int offset, int bytes, int value){
    for (int i = 0; i < bytes; i++){
	arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

bool write_image(string filename, const vector<vector<Pixel>>& image) {
    string fullPath = "../" + filename; // Uloženie do rodičovského priečinka

    int width_pixels = image[0].size();
    int height_pixels = image.size();
    int width_bytes = width_pixels * 3;
    int padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes += padding_bytes;
    int array_bytes = width_bytes * height_pixels;

    fstream stream;
    stream.open(fullPath, ios::out | ios::binary);

    if (!stream.is_open()) {
        cout << "Error: nemozeme vytvorit novy subor '" << fullPath << "'" << endl;
        return false;
    }

    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    set_bytes(bmp_header,  0, 1, 'B');
    set_bytes(bmp_header,  1, 1, 'M');
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE + array_bytes);
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE);

    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);
    set_bytes(dib_header,  4, 4, width_pixels);
    set_bytes(dib_header,  8, 4, height_pixels);
    set_bytes(dib_header, 12, 2, 1);
    set_bytes(dib_header, 14, 2, 24);
    set_bytes(dib_header, 20, 4, array_bytes);
    set_bytes(dib_header, 24, 4, 2835);
    set_bytes(dib_header, 28, 4, 2835);

    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    for (int h = height_pixels - 1; h >= 0; h--) {
        for (int w = 0; w < width_pixels; w++) {
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        stream.write((char*)padding, padding_bytes);
    }

    stream.close();
    cout << "obrazok sa ulozil do : " << fullPath << endl;
    return true;
}
vector<vector<Pixel> > convert_to_grayscale(const vector<vector<Pixel> >& image) {
    
    int width_pixels = image[0].size();
    int height_pixels = image.size();

     vector<vector<Pixel> > new_image(height_pixels, vector<Pixel> (width_pixels));

    for (int row = 0; row < height_pixels; row++) { 
        for (int col = 0; col < width_pixels; col++) {

            int red = image[row][col].red;
            int green = image[row][col].green;
            int blue = image[row][col].blue;

           	int gray_value = ((red + green + blue)/3) + 0.5;

      		new_image[row][col].red = gray_value; 
            new_image[row][col].green = gray_value;
            new_image[row][col].blue = gray_value;
        }
    }

  return new_image;
}

int main(){
    string inputFilename, outputFilename;
    cout << "napiste nazov suboru , ktory chcete preafarbit aj s priponou bmp (napr. vstup.bmp): ";
    cin >> inputFilename;
    cout << "napiste nazov  prefarbeneho suboru aj  s priponou bmp (napr. vystup.bmp):";
    cin >> outputFilename;
    
    vector<vector<Pixel> > image = read_image(inputFilename);
    
    if (image.empty()) {
        cout << "Error: nepodarilo sa otvoriť subor" << endl;
        return 1;
    }
    
	vector<vector<Pixel> > grayscaleImage = convert_to_grayscale(image);
    if (write_image(outputFilename, grayscaleImage)) {
        cout << "uspesne prefarbene!" << endl;
    } else {
        cout << "Error: nemozeme vytvorit  novy subor" << endl;
        return 1;
    }
    
    return 0;
}
