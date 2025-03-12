#include <iostream>
#include <opencv2/opencv.hpp>
#include <immintrin.h> // Include for AVX intrinsics
#include <chrono> // For timing

using namespace cv;
using namespace std;
using namespace std::chrono;

// Function to adjust brightness using standard OpenCV (baseline)
Mat adjustBrightnessOpenCV(const Mat& image, int brightness) {
    Mat brightened_image = image.clone();
    brightened_image.convertTo(brightened_image, -1, 1, brightness); // Simple OpenCV brightness adjustment
    return brightened_image;
}

Mat adjustBrightnessAVX2(const Mat& image, int brightness) {
    Mat brightened_image = image.clone();
    if (brightened_image.channels() != 3) {
        cerr << "Error: AVX2 brightness adjustment only works for 3-channel color images (BGR)." << endl;
        return brightened_image;
    }

    int rows = brightened_image.rows;
    int cols = brightened_image.cols;
    uchar* pixel_ptr = brightened_image.data;




    for (int i = 0; i < rows; ++i) {
        int j = 0; // Reset j for each row
        for (; j + 32 <= cols*3; j += 32) { // AVX loop (condition and increment in loop header)
            //DOPLN KOD

        }
        // Scalar processing for remaining pixels in the row (after AVX loop)
        for (int k = j/3; k < cols; ++k) { //Corrected loop index k and condition
            for (int c = 0; c < 3; ++c) {
                int pixel_value = pixel_ptr[(i * cols + k) * 3 + c];
                pixel_value += brightness;
                pixel_ptr[(i * cols + k) * 3 + c] = static_cast<uchar>(std::clamp(pixel_value, 0, 255));
            }
        }
    }
    return brightened_image;
}


// Declare the external assembly function
extern "C" {
    void brightness_asm(uchar* data, int width, int height, int brightness);

    void brightness_mmx(uchar* data, int width, int height, int brightness);
}


Mat adjustBrightnessMMX(const Mat& image, int brightness) {
    Mat brightened_image = image.clone();
    if (brightened_image.channels() != 3) {
        cerr << "Error: MMX brightness adjustment only works for 3-channel color images (BGR)." << endl;
        return brightened_image;
    }

    brightness_mmx(brightened_image.data, brightened_image.cols, brightened_image.rows, brightness);
    return brightened_image;
}

Mat adjustBrightnessASM(const Mat& image, int brightness) {
    Mat brightened_image = image.clone();
    if (brightened_image.channels() != 3) {
        cerr << "Error: ASM brightness adjustment only works for 3-channel color images (BGR)." << endl;
        return brightened_image;
    }

    brightness_asm(brightened_image.data, brightened_image.cols, brightened_image.rows, brightness);
    return brightened_image;
}


int main() {
    // Load an example image (replace with your image path)
    Mat image = imread("input/input.jpg");
    if (image.empty()) {
        cerr << "Error: Could not load image." << endl;
        return -1;
    }

    if (image.channels() != 3) {
        cvtColor(image, image, COLOR_GRAY2BGR); // Convert to BGR if grayscale
    }

    Size display_size(640, 480); // Desired display resolution

    int brightnessValue = 50; // Brightness adjustment value
    int num_iterations = 100; // Number of iterations for benchmarking

    long long total_duration_opencv = 0;
    long long total_duration_avx2 = 0;
    long long total_duration_mmx = 0;
	Mat brightenedASM, brightenedAVX2, brightenedMMX;


    // Warmup iterations (optional, to stabilize CPU frequency)
    for (int i = 0; i < 5; ++i) {
        adjustBrightnessOpenCV(image, brightnessValue);
        adjustBrightnessAVX2(image, brightnessValue);
        adjustBrightnessMMX(image, brightnessValue);
    }


    for (int i = 0; i < num_iterations; ++i) {
        // --- OpenCV Baseline ---
        auto start_opencv = high_resolution_clock::now();
        brightenedASM = adjustBrightnessASM(image, brightnessValue);
        auto stop_opencv = high_resolution_clock::now();
        auto duration_opencv = duration_cast<microseconds>(stop_opencv - start_opencv);
        total_duration_opencv += duration_opencv.count();

        // --- AVX2 Version ---
        auto start_avx2 = high_resolution_clock::now();
        brightenedAVX2 = adjustBrightnessAVX2(image, brightnessValue);
        auto stop_avx2 = high_resolution_clock::now();
        auto duration_avx2 = duration_cast<microseconds>(stop_avx2 - start_avx2);
        total_duration_avx2 += duration_avx2.count();

        // --- MMX Version ---
        auto start_mmx = high_resolution_clock::now();
        brightenedMMX = adjustBrightnessMMX(image, brightnessValue);
        auto stop_mmx = high_resolution_clock::now();
        auto duration_mmx = duration_cast<microseconds>(stop_mmx - start_mmx);
        total_duration_mmx += duration_mmx.count();
    }

    double avg_duration_opencv = static_cast<double>(total_duration_opencv) / num_iterations;
    double avg_duration_avx2 = static_cast<double>(total_duration_avx2) / num_iterations;
    double avg_duration_mmx = static_cast<double>(total_duration_mmx) / num_iterations;


    cout << "ASM Average Duration: " << avg_duration_opencv << " microseconds" << endl;
    cout << "AVX2 Average Duration:   " << avg_duration_avx2 << " microseconds" << endl;
    cout << "MMX Average Duration:    " << avg_duration_mmx << " microseconds" << endl;
    cout << "Speedup (OpenCV / AVX2): " << avg_duration_opencv / avg_duration_avx2 << "x" << endl;
    cout << "Speedup (OpenCV / MMX):  " << avg_duration_opencv / avg_duration_mmx << "x" << endl;

    // Resize images
    Mat resized_image, resized_opencv, resized_avx2, resized_mmx;
    resize(image, resized_image, display_size);
    resize(brightenedASM, resized_opencv, display_size);
    resize(brightenedAVX2, resized_avx2, display_size);
    resize(brightenedMMX, resized_mmx, display_size);

    // Add text to images
    putText(resized_image, "Original", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
    putText(resized_opencv, "ASM", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
    putText(resized_avx2, "AVX2", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
    putText(resized_mmx, "MMX", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);

    // Combine images into 2x2 grid
    Mat row1, row2, combined_image;
    hconcat(resized_image, resized_opencv, row1);
    hconcat(resized_avx2, resized_mmx, row2);
    vconcat(row1, row2, combined_image);


    // Optionally display the results (for visual verification)
    imshow("Original | ASM | AVX2 | MMX Brightened", combined_image);
    waitKey(0);
    destroyAllWindows();

    return 0;
}