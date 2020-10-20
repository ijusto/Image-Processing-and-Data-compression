/*!
 * Calculates the histogram of an audio sample and corresponding entropy.
 * Calculates the histogram of the left and right channels, as well as the histogram of the average of the channels (the
 * mono version). Note: For visualizing graphically the histograms, you can either save the histogram data as a text
 * file and use an external application to visualize it or you can extend the functionality of the program in order to
 * graphically display the histogram.
 * @author Inês Justo
 * @author
 */

// TODO: Everything xd
#include "thirdparty/matplotlib-cpp-master/matplotlibcpp.h"
namespace plt = matplotlibcpp;
int main() {
    plt::plot({1,3,2,4});
    plt::show();
}