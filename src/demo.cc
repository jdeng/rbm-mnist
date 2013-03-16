#include "mnist.h"
#include "rbm.h"

/** Algorithm from Earl F. Glynn's web page:
* <a href="http://www.efg2.com/Lab/ScienceAndEngineering/Spectra.htm">Spectra Lab Report</a>
* */
#include "spectrum.inl"

#include <Magick++.h>

// MacOS X
// clang++ -g -O2 -std=c++11 -stdlib=libc++ -I/usr/local/include/GraphicsMagick -o dbn demo.cc /Users/jack/Downloads/GraphicsMagick-1.3.18/Magick++/lib/.libs/libGraphicsMagick++.a  /Users/jack/Downloads/GraphicsMagick-1.3.18/magick/.libs/libGraphicsMagick.a -lz -lxml2 -lpng -ljpeg -lbz2
// Ubuntu
// clang++ --std=c++0x -o dbn -g -O2 demo.cc -lz -I /usr/include/GraphicsMagick -lGraphicsMagick++

int main(int argc, const char *argv[])
{
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << "<train-simple|train|test> <image-file> <label-file>" << std::endl;
		return -1;
	}

	std::vector<Sample> samples;
	int n = mnist::read(argv[2], argv[3], samples);
	if (n <= 0) {
		std::cerr << "failed to read mnist data files: " << argv[2] << " ," << argv[3] << std::endl;
		return -1;
	}

	std::string command = argv[1];

	// initialize pallet
	Magick::InitializeMagick(*argv);
	std::vector<Magick::Color> pallet;
	for (auto& rgb: _pallet) pallet.push_back(Magick::Color(rgb.r, rgb.g, rgb.b));

	// initialize data
	int data_size = samples[0].data_.size();
	std::vector<RealVector> inputs(n);
	std::vector<RealVector> targets(n);
	for (size_t i=0; i< n; ++i) {
		const Sample& sample = samples[i];
		RealVector& input = inputs[i];
		RealVector& target = targets[i];

		input.resize(data_size); target.resize(10);
		for (size_t j=0; j<data_size; ++j) input[j] = sample.data_[j] / 255.0; // > 30 ? 1.0: 0.0; // binary 
		target[sample.label_] = 1.0;
	}

	// progress monitoring
	auto progress = [&pallet](DeepBeliefNet& dbn) {
		static int i = 0;
		int width = 0, height = 0;
		RealVector pixels;
		dbn.to_image(pixels, width, height);

		Magick::Image img(Magick::Geometry(width * 2, height * 2), Magick::Color(255,255,255));
		for (size_t x=0; x < width * 2; ++x) {
				for (size_t y=0; y < height * 2; ++y) {
					int i =  int(abs(pixels[int(y / 2 * width + x / 2)] * 255));
					if (i > 255 || i < 0) i = 255;
					img.pixelColor(x, y, pallet[i]);
				}
		}
		std::string name = "rbm-" + std::to_string(i++);
		std::string fn = name + ".png";
		img.write(fn.c_str());

		std::ofstream f(name + ".dat", std::ofstream::binary);
		dbn.store(f);
	};
	
	// training and testing functions
	auto train_dbn_simple = [&]() {
		DeepBeliefNet dbn;
		dbn.build(std::vector<int>{data_size, 300, 300, 500}, std::vector<int>{0, 0, 10});

		LRBM::Conf conf;
		conf.max_epoch_ = 6; conf.max_batches_ = 100; conf.batch_size_ = 100;
		
		dbn.train(inputs, targets, dbn.max_layer(), conf, progress);
	
		std::ofstream f("dbn-s.dat", std::ofstream::binary);
		dbn.store(f);
	};
	
	auto train_dbn = [&]() {
		DeepBeliefNet dbn;

		dbn.build(std::vector<int>{data_size, 300, 300, 500, 10});
		auto& rbm = dbn.output_layer();
		rbm->type_ = RBM::Type::EXP;

  	std::default_random_engine eng(::time(NULL));
  	std::normal_distribution<double> rng(0.0, 1.0);

		LRBM::Conf conf;

		bool resume = false;
		if (resume) {
			std::ifstream f("dbn.dat", std::ifstream::binary);
			dbn.load(f);
		}
		else {
			conf.max_epoch_ = 10; conf.max_batches_ = 50; conf.batch_size_ = 100;
			dbn.pretrain(inputs, conf, progress);
		}

		conf.max_epoch_ = 10; conf.max_batches_ /= 5; conf.batch_size_ *= 5;
		dbn.backprop(inputs, targets, conf, progress);

		std::ofstream f("dbn.dat", std::ofstream::binary);
		dbn.store(f);
	};

	auto train_autoencoder = [&]() {
		AutoEncoder enc;
		enc.build(std::vector<int>{data_size, 500, 30, 500, data_size});

		auto& rbm = enc.rbms_[enc.max_layer() / 2 - 1];
		rbm->type_ = RBM::Type::LINEAR;

		LRBM::Conf conf;
		conf.max_epoch_ = 10; conf.max_batches_ = 50; conf.batch_size_ = 100;
		enc.pretrain(inputs, conf, progress);

		conf.max_epoch_ = 10; conf.max_batches_ /= 5; conf.batch_size_ *= 5;
		enc.backprop(inputs, conf, progress);

		std::ofstream f("enc.dat", std::ofstream::binary);
		enc.store(f);
	};

	auto test_dbn = [&](bool is_simple) {
		DeepBeliefNet rbm;
		std::string file = is_simple? "dbn-s.dat" : "dbn.dat";
		std::ifstream f(file, std::ifstream::binary);
		rbm.load(f);

		size_t correct = 0, second = 0;
		for (size_t i = 0; i < samples.size(); ++i) {
			const Sample& sample = samples[i];

			std::vector<int> idx(10);
			for(int i=0; i<10; ++i) idx[i] = i;

			RealVector output(10);
			if (is_simple)
				rbm.predict(inputs[i], output, RealVector::nil());
			else
				rbm.predict(inputs[i], RealVector::nil(), output);

			std::sort(idx.begin(), idx.end(), [&output](int x, int y) { return output[x] > output[y]; });

			if (idx[0] == (int)sample.label_) ++ correct;
			else if (idx[1] == (int)sample.label_) ++ second;


			if ((i + 1) % 100 == 0)	std::cout << "# " << correct << "/" << i + 1 << " recognized. 1st: " << (correct * 100.0/ (i+1)) << "%, 1st+2nd: " << (correct + second) * 100.0/(i+1) << "%" << std::endl;
		}

		std::cout << "# " << correct << " recognized." << std::endl;
	};

	// execute commands
	if (command == "train") train_dbn();
	else if (command == "train-simple") train_dbn_simple();
	else if (command == "test") test_dbn(false);
	else if (command == "test-simple") test_dbn(true);
	else if (command == "train-encoder") train_autoencoder();
	else {
		std::cerr << "unrecognized command: " << command << std::endl;	
	}

	return 0;
}

