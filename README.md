# Source Code
The deep learning algorithm is based on the matlab code provided by Geoff Hinton etc at http://www.cs.toronto.edu/~hinton/MatlabForSciencePaper.html

The Conjugate Gradient implementation is based (translated and simplfied) on Carl Edward Rasmussen's matlab code at http://learning.eng.cam.ac.uk/carl/code/minimize/minimize.m

* rbm.h the DBN/Autoencoder implementation.
* mnist.h Reading MNIST data file
* spectrum.inl RGBs to map weights to colors. Blue is 0 and red is close to 1.0.
* demo.cc training/test demos

It is done with two weekends so there will be bugs and defects. 

# Visualization
It is not trivial to monitor an optimization procedure with 1 million parameters or more. One way is to map the weights to colors (assuming most of the weights are within [-1,1]) and show them together as an image. Input is on Y-axis (rows) and output is on X-axis (columns). There are 4 RBMs in the sample image (rbm-131.png): 784->300, 300->300, 300->500, 500->10. Check out the images periodically and you can have a rought idea whether the parameters look right.

# Building

C++ 11 is extensivily used and currently only clang 3.1 is tested for building. GraphicsMagick is used to generate the representation and is the only dependency.

Under MacOS X, GraphicsMagick installed by brew probably won't work due to libc++/libstdc++ subtleties. But you can use it to install the header files and dependent libraries like libjpeg, libpng, etc.
Then you can build a private static GraphicMagick library with:

`CC=clang CXX=clang++ CXXFLAGS="-stdlib=libc++" ./configure --enable-static --disable-shared --disable-openmp`

After that you can build dbn with something like below (replace the path to GrphicsMagick accordingly)
`clang++ -g -O2 -std=c++11 -stdlib=libc++ -I/usr/local/include/GraphicsMagick -o dbn demo.cc /Users/jack/Downloads/GraphicsMagick-1.3.18/Magick++/lib/.libs/libGraphicsMagick++.a  /Users/jack/Downloads/GraphicsMagick-1.3.18/magick/.libs/libGraphicsMagick.a -lz -lxml2 -lpng -ljpeg -lbz2`

Under Ubuntu it is much easier thanks to the new version of libstdc++. Once you have clang 3.1 installed (Here is a source: http://askubuntu.com/questions/141597/llvm-clang-3-1-libc), you can build the software with

`clang++ --std=c++0x -o dbn -g -O2 demo.cc -lz -I /usr/include/GraphicsMagick -lGraphicsMagick++`

You'll also need to install GraphicsMagick probably with apt-get before that.

# Train and Test
The data fiels are available at http://yann.lecun.com/exdb/mnist/.

The command line looks like: ./dbn <command> <path-to-mnist-image-file> <path-to-mnist-label-file> where command could be "train", "train-simple", "test", "test-simple", "train-encoder" etc.
It is highly recommended that you read through demo.cc before running tests.

There are 3 types of topology: simple DBN, fine tuned DBN and Autoencoder.
 * Simple DBN: the labels are attached at the last layer. Use "train-simple" and "test-simple" to train/test.
 * Fine tuned DBN: the labels are trained as the output of last layer with Conjugate Gradient after the structure of other RBMs has been trained. Use "train" and "test" to train/test.
 * Autoencoder: not tested yet.

The default monitoring function (progress in demo.cc) will generate a snapshot of the DBN periodically with a rbm-<n>.png and rbm-<n>.dat file. The png file shows the weight changes in a straightforward way. The .dat file can be used for testing with renaming to the correct name (e.g., dbn.dat).

# Performance
There is no extensive testing result yet. Below are some intial numbers for your information based on training with first half of the 10k testing dataset witha few epoches(< 10). Testing is carried out on the whole 10k dataset. It takes about an hour to train. Testing is fast.

* Simple DBN: ~87%.
* Fine tuned DBN: on the 5000 training set, 100%; on the whole set, ~98%.

The improvement from the CG fine tuning is obvious. It would not be difficult to tune and reproduce the same results in Hinton's paers.

During the tests it turns out that the topology (numbers of hidden/visible units) does not impact much on the performance.
