# Build

C++ 11 is extensivily used and currently only clang 3.1 is tested for building. GraphicsMagick is used to generate the representation and is the only dependency.

Under MacOS X, GraphicsMagick installed by brew probably won't work due to libc++/libstdc++ subtleties. But you can use it to install the header files and dependent libraries like libjpeg, libpng, etc.
Then you can build a private static GraphicMagick library with:

CC=clang CXX=clang++ CXXFLAGS="-stdlib=libc++" ./configure --enable-static --disable-shared --disable-openmp

After that you can build dbn with something like below (replace the path to GrphicsMagick accordingly)
clang++ -g -O2 -std=c++11 -stdlib=libc++ -I/usr/local/include/GraphicsMagick -o dbn demo.cc /Users/jack/Downloads/GraphicsMagick-1.3.18/Magick++/lib/.libs/libGraphicsMagick++.a  /Users/jack/Downloads/GraphicsMagick-1.3.18/magick/.libs/libGraphicsMagick.a -lz -lxml2 -lpng -ljpeg -lbz2

Under Ubuntu it is much easier thanks to the new version of libstdc++. Once you have clang 3.1 installed (Here is a source: http://askubuntu.com/questions/141597/llvm-clang-3-1-libc), you can build the software with
clang++ --std=c++0x -o dbn -g -O2 demo.cc -lz -I /usr/include/GraphicsMagick -lGraphicsMagick++

You'll also need to install GraphicsMagick with apt-get before that.

# Train and Test
The command line looks like: ./dbn <command> <path-to-mnist-image-file> <path-to-mnist-label-file> where command could be "train", "train-simple", "test", "test-simple", "train-encoder" etc.
It is highly recommended that you read through demo.cc before running tests.

There are 3 types of topology: simple DBN, fine tuned DBN and Autoencoder.
 * Simple DBN: the labels are attached at the last layer. Use "train-simple" and "test-simple" to train/test.
 * Fine tuned DBN: the labels are trained as the output of last layer with Conjugate Gradient after the structure of other RBMs has been trained. Use "train" and "test" to train/test.
 * Autoencoder: not tested yet.
