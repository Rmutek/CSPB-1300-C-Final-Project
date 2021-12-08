/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Rebecca Mutek

- All project requirements fully met? (YES or NO):
    Yes

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
//

/**
 * Adds vignette effect - dark corners
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_1(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            // find the distance to the center
            double distance = sqrt(pow((col - num_columns/2), 2) + pow((row - num_rows/2),2));
            double scaling_factor = (num_rows - distance)/num_rows;

            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int newred = red_color * scaling_factor;
            int newgreen = green_color * scaling_factor;
            int newblue = blue_color * scaling_factor;
            
            // Set the blue color for the Pixel located at index row, col in a new 2D vector to 50
            new_image[row][col].red = newred;
            new_image[row][col].green = newgreen;
            new_image[row][col].blue = newblue;
        }
    }

    return new_image;
}

/**
 * Adds clarendon type effect - darks darker and lights lighter
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_2(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    double scaling_factor = 0.3;

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int average_color_value = (red_color + green_color + blue_color)/3;

            int newred = 0;
            int newgreen = 0;
            int newblue = 0;

            if (average_color_value >= 170)
            {
                newred = int(255 - (255 - red_color)*scaling_factor);
                newgreen = int(255 - (255 - green_color)*scaling_factor);
                newblue =  int(255 - (255 - blue_color)*scaling_factor);
            } 
            else if (average_color_value < 90)
            {
                newred = red_color * scaling_factor;
                newgreen = green_color * scaling_factor;
                newblue = blue_color * scaling_factor;
            }
            else
            {
                newred = red_color;
                newgreen = green_color;
                newblue = blue_color;
            }
            
            // Set the blue color for the Pixel located at index row, col in a new 2D vector to 50
            new_image[row][col].red = newred;
            new_image[row][col].green = newgreen;
            new_image[row][col].blue = newblue;
        }
    }

    return new_image;
}

/**
 * Greyscale the image
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_3(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int gray_value = (red_color + green_color + blue_color)/3;
            
            new_image[row][col].red = gray_value;
            new_image[row][col].green = gray_value;
            new_image[row][col].blue = gray_value;
        }
    }

    return new_image;
}

/**
 * Rotates by 90 degrees
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
// vector<vector<Pixel>> process_4(vector<vector<Pixel>> image) {
//     int num_rows = image.size(); // height
//     int num_columns = image[0].size(); // width
//     vector<vector<Pixel>> new_image(num_columns, vector<Pixel> (num_rows));

//     for (int row = 0; row < num_rows; row++)
//     {
//         for (int col = 0; col < num_columns; col++)
//         {
//             int red_color = image[row][col].red;
//             int green_color = image[row][col].green;
//             int blue_color = image[row][col].blue;
            
//             new_image[col][row-1].red = red_color;
//             new_image[col][row-1].green = green_color;
//             new_image[col][row-1].blue = blue_color;
//         }
//     }

//     // for (int col = num_columns; col > 0; col--)
//     // {
//     //     for (int row = 0; row < num_rows; row++)
//     //     {
//     //         int red_color = image[col][row].red;
//     //         int green_color = image[col][row].green;
//     //         int blue_color = image[col][row].blue;

//     //         new_image[col][row].red = red_color;
//     //         new_image[col][row].green = green_color;
//     //         new_image[col][row].blue = blue_color;
//     //     }
//     // }

//     return new_image;
// }

/**
 * Rotates by 90 degrees
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_4(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    int new_rows = num_columns;
    int new_columns = num_rows;
    vector<vector<Pixel>> new_image(new_rows, vector<Pixel> (new_columns));
    vector<vector<Pixel>> new_image2(new_rows, vector<Pixel> (new_columns));

    for (int row = 0; row < new_rows; row++)
    {
        for (int col = 0; col < new_columns; col++)
        {
            int red_color = image[col][row].red;
            int green_color = image[col][row].green;
            int blue_color = image[col][row].blue;

            new_image[row][col].red = red_color;
            new_image[row][col].green = green_color;
            new_image[row][col].blue = blue_color;
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_4_2(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {

            int red_color = image[row][num_columns - col].red;
            int green_color = image[row][num_columns - col].green;
            int blue_color = image[row][num_columns - col].blue;

            new_image[row][col].red = red_color;
            new_image[row][col].green = green_color;
            new_image[row][col].blue = blue_color;
        }
    }

    return new_image;
}



/**
 * Rotates by multiples of 90 degrees
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_5(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            new_image[row][col].red = (num_rows-1);
            new_image[row][col].green = (num_rows-1);
            new_image[row][col].blue = (num_rows-1);
        }
    }

    return new_image;
}

/**
 * Enlarges in the x and y direction
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_6(vector<vector<Pixel>> image, int x, int y) {
    int num_rows = image.size(); // height
    int new_rows = num_rows * y;
    int num_columns = image[0].size(); // width
    int new_columns = num_columns * x; // width
    vector<vector<Pixel>> new_image(new_rows, vector<Pixel> (new_columns));

    for (int row = 0; row < new_rows; row++)
    {
        for (int col = 0; col < new_columns; col++)
        {
            int colx = col/x;
            int rowy = row/y;

            int red_color = image[rowy][colx].red;
            int green_color = image[rowy][colx].green;
            int blue_color = image[rowy][colx].blue;

            new_image[row][col].red = red_color;
            new_image[row][col].green = green_color;
            new_image[row][col].blue = blue_color;
        }
    }

    return new_image;
}

/**
 * Converts image to high contrast - black and white only
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_7(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int gray_value = (red_color + green_color + blue_color)/3;

            int newred = 0;
            int newgreen = 0;
            int newblue = 0;

            if (gray_value >= 255/2)
            {
                newred = 255;
                newgreen = 255;
                newblue = 255;
            }
            
            new_image[row][col].red = newred;
            new_image[row][col].green = newgreen;
            new_image[row][col].blue = newblue;
        }
    }

    return new_image;
}

/**
 * Lightens image
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_8(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            double scaling_factor = 0.6;

            int newred = 255 - (255 - red_color) * scaling_factor;
            int newgreen = 255 - (255 - green_color) * scaling_factor;
            int newblue = 255 - (255 - blue_color) * scaling_factor;
            
            new_image[row][col].red = newred;
            new_image[row][col].green = newgreen;
            new_image[row][col].blue = newblue;
        }
    }

    return new_image;
}

/**
 * Darkens image
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_9(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            double scaling_factor = 0.55;

            int newred = red_color * scaling_factor;
            int newgreen = green_color * scaling_factor;
            int newblue = blue_color * scaling_factor;
            
            new_image[row][col].red = newred;
            new_image[row][col].green = newgreen;
            new_image[row][col].blue = newblue;
        }
    }

    return new_image;
}

/**
 * Converts to only black, white, red, blue and green
 * @param image The input image to add effect to
 * @return vector of vectors of type Pixel
 */
vector<vector<Pixel>> process_10(vector<vector<Pixel>> image) {
    int num_rows = image.size(); // height
    int num_columns = image[0].size(); // width
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int newred = 0;
            int newgreen = 0;
            int newblue = 0;

            int max_color = red_color;
            if (green_color > red_color && green_color > blue_color)
            {
                max_color = green_color;
            }
            else if (blue_color > red_color && blue_color > green_color)
            {
                max_color = blue_color;
            }

            if (red_color + green_color + blue_color >= 550)
            {
                newred = 255;
                newgreen = 255;
                newblue = 255;
            }
            else if (red_color + green_color + blue_color <= 150)
            {
                newred = 0;
                newgreen = 0;
                newblue = 0;
            }
            else if (max_color == red_color)
            {
                newred = 255;
                newgreen = 0;
                newblue = 0;
            }
            else if (max_color == green_color)
            {
                newred = 0;
                newgreen = 255;
                newblue = 0;
            }
            else
            {
                newred = 0;
                newgreen = 0;
                newblue = 255;
            }
            
            new_image[row][col].red = newred;
            new_image[row][col].green = newgreen;
            new_image[row][col].blue = newblue;
        }
    }

    return new_image;
}

int main()
{
    
    // Read in BMP image file into a 2D vector (using read_image function)
    vector<vector<Pixel>> img = read_image("sample2.bmp");

    // Call process_1 function using the input 2D vector and save the result returned to a new 2D vector
    vector<vector<Pixel>> img_process_1 = process_1(img);
    write_image("test_sample_process_1.bmp", img_process_1);

    vector<vector<Pixel>> img_process_2 = process_2(img);
    write_image("test_sample_process_2.bmp", img_process_2);

    vector<vector<Pixel>> img_process_3 = process_3(img);
    write_image("test_sample_process_3.bmp", img_process_3);

    vector<vector<Pixel>> img_process_4 = process_4(img);

    vector<vector<Pixel>> img_process_4_2 = process_4_2(img_process_4);
    write_image("test_sample_process_4.bmp", img_process_4_2);

    // TODO
    vector<vector<Pixel>> img_process_5 = process_5(img);
    write_image("test_sample_process_5.bmp", img_process_5);

    int x = 1;
    int y = 4;
    vector<vector<Pixel>> img_process_6 = process_6(img, x, y);
    write_image("test_sample_process_6.bmp", img_process_6);

    vector<vector<Pixel>> img_process_7 = process_7(img);
    write_image("test_sample_process_7.bmp", img_process_7);

    vector<vector<Pixel>> img_process_8 = process_8(img);
    write_image("test_sample_process_8.bmp", img_process_8);

    vector<vector<Pixel>> img_process_9 = process_9(img);
    write_image("test_sample_process_9.bmp", img_process_9);

    vector<vector<Pixel>> img_process_10 = process_10(img);
    write_image("test_sample_process_10.bmp", img_process_10);

    // Write the resulting 2D vector to a new BMP image file (using write_image function)
	cout << "\n\n\nThis line should be your own code!\n\n\n";

    return 0;
}