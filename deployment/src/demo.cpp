#ifndef YOLO_DEMO_CPP
#define YOLO_DEMO_CPP

#include "../yolo/detect.hpp"
#include "../yolo/os.h"
#include "../yolo/datetime.h"
#include "./argparser.h"


argparser::ArgumentParser parseArgs(int argc, char** argv)
{
    argparser::ArgumentParser parser("yolo demo parser", argc, argv);

    parser.add_option<std::string>("-c", "--config", "config file path", "");
    parser.add_option<std::string>("-s", "--source", "video source", "");
    parser.add_option<bool>("-p", "--pause", "stop at start", false);
    parser.add_help_option();

    return parser.parse();
}


int main(int argc, char** argv)
{
    auto args = parseArgs(argc, argv);

    pystring configPath = args.get_option_string("--config");
    pystring sourcePath = args.get_option_string("--source");


    if (!configPath.lower().endswith(".yaml") && !configPath.lower().endswith("yml"))
    {
        std::cerr << "[E] only support yaml config file, got " << configPath.lower() << std::endl;
        return -1;
    }
    if (!os::path::isfile(configPath))
    {
        std::cerr << "[E] config file not exist!" << std::endl;
        return -1;
    }

    std::cout << "----------------start init detector-----------------" << std::endl;


    Detector det;
    if(!det.init(configPath.str()))
    {
        std::cerr << "[E] failed to init detector!"  << std::endl;
        return -1;
    }

    std::cout << "\033[32m\033[1m[INFO] acceleration platform: " 
              << det.platform() << "\033[0m" << std::endl;
    
    
    
    std::cout << "--------------------start detect--------------------" << std::endl;


    cv::VideoCapture cap(sourcePath.str());
    int delay = args.get_option_bool("--pause")?0:1;
    cv::Mat frame;
    std::vector<std::vector<float>> result;


    while (cap.isOpened())
    {
        cap.read(frame);
        if (frame.empty())
        {
            std::cerr << "\nvideo is empty, exit.";
            cap.release();
            break;
        }
        
        result.clear();

        float ratio=1.0;
        cv::Mat resizedImage;

        double t0 = pytime::time();
        // ----------- detect -----------
        det.detect(frame, result);

        
        // det.preProcess(frame, resizedImage, ratio);

        // double t1 = pytime::time();

        // det.infer(resizedImage, ratio, result);

        double t2 = pytime::time();

        // ------------------------------
        // std::cerr << "\rdelay: " << std::fixed << std::setprecision(3) << 1000 * (t2 - t0) << "ms, "
        //           << " preprocess:" << 1000 * (t1 - t0) << "ms,  infer:" << 1000 * (t2 - t1) << " ms.   ";

        std::cerr << "\rdelay: " << std::fixed << std::setprecision(3) << 1000 * (t2 - t0) << "ms  ";

        cv::Mat frame_show;
        if (result.size())
        {
            frame_show = det.draw(frame, result);
            // cv::imwrite("test.jpg", frame);
        }

        cv::imshow("demo", frame_show);
        int k = cv::waitKey(delay);
        if (k == ' ')
        {
            delay = 1 - delay;
        }
        else if (k == 27)
        {
            cap.release();
            break;
        }
    }
    std::cout << std::endl;

    cv::destroyAllWindows();
    return 0;
}

#endif