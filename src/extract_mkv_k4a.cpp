//
// Created by narvis on 04.07.19.
//

// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015-2017 Intel Corporation. All Rights Reserved.

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams
#include <exception>
#include <algorithm>

#include <Corrade/configure.h>
#include <Corrade/Utility/Arguments.h>
#include <Corrade/Utility/Debug.h>
#include <Corrade/Utility/DebugStl.h>
#include <Corrade/Utility/Directory.h>
#include <Corrade/Containers/ArrayView.h>

#ifdef CORRADE_TARGET_UNIX
#ifdef CORRADE_TARGET_APPLE
#include <Magnum/Platform/WindowlessCglApplication.h>
#else

#include <Magnum/Platform/WindowlessGlxApplication.h>

#endif
#else
#ifdef CORRADE_TARGET_WINDOWS
#include <Magnum/Platform/WindowlessWglApplication.h>
#endif
#endif

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/ArrayView.h>

#include "Magnum/Math/Matrix3.h"
#include "Magnum/Math/Matrix4.h"
#include "Magnum/Math/Range.h"

#include "Magnum/GL/Context.h"


#include <k4a/k4a.hpp>
#include <k4arecord/playback.hpp>
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <opencv2/imgcodecs.hpp>   // Include OpenCV API

#include "transformation_helpers.h"

namespace Magnum {

    struct MissingDataException : public std::exception
    {
      const char * what () const throw ()
        {
            return "Failed to acquire data from the k4a_capture";
        }
    };

    static void print_calibration(k4a_calibration_t& calibration)
    {
        using namespace std;

        {
            cout << "Depth camera:" << endl;
            auto calib = calibration.depth_camera_calibration;

            cout << "resolution width: " << calib.resolution_width << endl;
            cout << "resolution height: " << calib.resolution_height << endl;
            cout << "principal point x: " << calib.intrinsics.parameters.param.cx << endl;
            cout << "principal point y: " << calib.intrinsics.parameters.param.cy << endl;
            cout << "focal length x: " << calib.intrinsics.parameters.param.fx << endl;
            cout << "focal length y: " << calib.intrinsics.parameters.param.fy << endl;
            cout << "radial distortion coefficients:" << endl;
            cout << "k1: " << calib.intrinsics.parameters.param.k1 << endl;
            cout << "k2: " << calib.intrinsics.parameters.param.k2 << endl;
            cout << "k3: " << calib.intrinsics.parameters.param.k3 << endl;
            cout << "k4: " << calib.intrinsics.parameters.param.k4 << endl;
            cout << "k5: " << calib.intrinsics.parameters.param.k5 << endl;
            cout << "k6: " << calib.intrinsics.parameters.param.k6 << endl;
            cout << "center of distortion in Z=1 plane, x: " << calib.intrinsics.parameters.param.codx << endl;
            cout << "center of distortion in Z=1 plane, y: " << calib.intrinsics.parameters.param.cody << endl;
            cout << "tangential distortion coefficient x: " << calib.intrinsics.parameters.param.p1 << endl;
            cout << "tangential distortion coefficient y: " << calib.intrinsics.parameters.param.p2 << endl;
            cout << "metric radius: " << calib.intrinsics.parameters.param.metric_radius << endl;
        }

        {
            cout << "Color camera:" << endl;
            auto calib = calibration.color_camera_calibration;

            cout << "resolution width: " << calib.resolution_width << endl;
            cout << "resolution height: " << calib.resolution_height << endl;
            cout << "principal point x: " << calib.intrinsics.parameters.param.cx << endl;
            cout << "principal point y: " << calib.intrinsics.parameters.param.cy << endl;
            cout << "focal length x: " << calib.intrinsics.parameters.param.fx << endl;
            cout << "focal length y: " << calib.intrinsics.parameters.param.fy << endl;
            cout << "radial distortion coefficients:" << endl;
            cout << "k1: " << calib.intrinsics.parameters.param.k1 << endl;
            cout << "k2: " << calib.intrinsics.parameters.param.k2 << endl;
            cout << "k3: " << calib.intrinsics.parameters.param.k3 << endl;
            cout << "k4: " << calib.intrinsics.parameters.param.k4 << endl;
            cout << "k5: " << calib.intrinsics.parameters.param.k5 << endl;
            cout << "k6: " << calib.intrinsics.parameters.param.k6 << endl;
            cout << "center of distortion in Z=1 plane, x: " << calib.intrinsics.parameters.param.codx << endl;
            cout << "center of distortion in Z=1 plane, y: " << calib.intrinsics.parameters.param.cody << endl;
            cout << "tangential distortion coefficient x: " << calib.intrinsics.parameters.param.p1 << endl;
            cout << "tangential distortion coefficient y: " << calib.intrinsics.parameters.param.p2 << endl;
            cout << "metric radius: " << calib.intrinsics.parameters.param.metric_radius << endl;
        }

        auto extrinsics = calibration.extrinsics[K4A_CALIBRATION_TYPE_DEPTH][K4A_CALIBRATION_TYPE_COLOR];
        cout << "depth2color translation: (" << extrinsics.translation[0] << "," << extrinsics.translation[1] << "," << extrinsics.translation[2] << ")" << endl;
        cout << "depth2color rotation: |" << extrinsics.rotation[0] << "," << extrinsics.rotation[1] << "," << extrinsics.rotation[2] << "|" << endl;
        cout << "                      |" << extrinsics.rotation[3] << "," << extrinsics.rotation[4] << "," << extrinsics.rotation[5] << "|" << endl;
        cout << "                      |" << extrinsics.rotation[6] << "," << extrinsics.rotation[7] << "," << extrinsics.rotation[8] << "|" << endl;

    }

    class ExtractFramesMKV : Platform::WindowlessApplication {
    public:
        explicit ExtractFramesMKV(const Arguments &arguments);

        int exec() override;

        void save_calibration(k4a_calibration_t);

        void process_depth(k4a::capture, int);
        void process_color(k4a::capture, int);
        void process_ir(k4a::capture, int);
        void process_rgbd(k4a::capture, int, k4a_calibration_t);

    private:
        k4a::playback m_dev;
        k4a_record_configuration_t m_dev_config;
        Corrade::Utility::Arguments args;
        std::string m_input_filename;
        std::string m_output_directory;
        std::ostringstream m_tsss;

        size_t m_first_frame{0};
        size_t m_last_frame{0};

        bool m_export_timestamp{false};
        bool m_export_color{false};
        bool m_export_depth{false};
        bool m_export_infrared{false};
        bool m_export_pointcloud{false};
        bool m_export_rgbd{false};

    };

    ExtractFramesMKV::ExtractFramesMKV(const Arguments &arguments) : Magnum::Platform::WindowlessApplication{
            arguments} {
        args.addArgument("input").setHelp("input", "Input file")
                .addOption("output-dir", "../test_data").setHelp("output-dir", "Output directory", "DIR")
                .addOption("first-frame", "0").setHelp("first-frame", "First frame to export")
                .addOption("last-frame", "0").setHelp("last-frame", "Last frame to export")
                .addBooleanOption("timestamp").setHelp("timestamp", "Export Timestamps to file")
                .addBooleanOption("color").setHelp("color", "Export Color stream")
                .addBooleanOption("depth").setHelp("depth", "Export Depth stream")
                .addBooleanOption("infrared").setHelp("infrared", "Export Infrared stream")
                .addBooleanOption("pointcloud").setHelp("pointcloud", "Export Pointcloud stream")
                .addBooleanOption("rgbd").setHelp("rgbd", "Export RGBD stream")
                .addSkippedPrefix("magnum", "engine-specific options");

        args.parse(arguments.argc, arguments.argv);

        m_input_filename = args.value("input");
        m_output_directory = args.value("output-dir");

        m_first_frame = args.value<Magnum::Int>("first-frame");
        m_last_frame = args.value<Magnum::Int>("last-frame");

        m_export_timestamp = args.isSet("timestamp");
        m_export_color = args.isSet("color");
        m_export_depth = args.isSet("depth");
        m_export_infrared = args.isSet("infrared");
        m_export_pointcloud = args.isSet("pointcloud");
        m_export_rgbd = args.isSet("rgbd");
    }

    int ExtractFramesMKV::exec() {
        Debug{} << "Core profile:" << GL::Context::current().isCoreProfile();
        Debug{} << "Context flags:" << GL::Context::current().flags();

        if (!Corrade::Utility::Directory::exists(m_input_filename)) {
            Magnum::Error{} << "Input file: " << m_input_filename << " not found!";
            return 1;
        }

        if (!Corrade::Utility::Directory::exists(m_output_directory)) {
            Magnum::Error{} << "Output directory: " << m_output_directory << " does not exist!";
            return 1;
        }

        if (!m_export_timestamp && !m_export_infrared && !m_export_color && !m_export_depth && !m_export_pointcloud && !m_export_rgbd) {
            Magnum::Error{} << "Error: No stream was selected for export!";
            Magnum::Debug{} << args.help();
            return 1;
        }

        m_dev = k4a::playback::open(m_input_filename.c_str());
//        if (m_export_pointcloud) {
//            Magnum::Debug{} << "Set color conversion to BGRA32 for pointcloud export";
//            m_dev.set_color_conversion(K4A_IMAGE_FORMAT_COLOR_BGRA32);
//        }
        m_dev_config = m_dev.get_record_configuration();

        // store calibration
        k4a_calibration_t calibration = m_dev.get_calibration();

        print_calibration(calibration);

        save_calibration(calibration);

        std::string timestamp_path = Corrade::Utility::Directory::join(m_output_directory, "timestamp.csv");

        if (m_export_timestamp) {
            Corrade::Utility::Directory::writeString(timestamp_path, "frameindex,depth_dts,depth_sts,color_dts,color_sts,infrared_dts,infrared_sts\n");
        }

        // now export frames

        k4a::capture capture;
        int frame_counter{0};

        for (;;) {
            try {
                if (m_dev.get_next_capture(&capture)) {

                    if (frame_counter < m_first_frame) {
                        frame_counter++;
                        continue;
                    }
                    if (m_last_frame > 0 && frame_counter > m_last_frame) {
                        break;
                    }

                    // record frameindex
                    m_tsss << std::to_string(frame_counter) << ",";

                    Magnum::Debug{} << "Extract Frame: " << frame_counter;

                    try {

                        process_depth(capture, frame_counter);

                        process_color(capture, frame_counter);

                        process_ir(capture, frame_counter);

                        process_rgbd(capture, frame_counter, calibration);

                    } catch (const Magnum::MissingDataException& e) {
                            Magnum::Error{} << "Error during playback: " << e.what();
                            continue;
                    }

                    if (m_export_timestamp) {
                        Corrade::Utility::Directory::appendString(timestamp_path, m_tsss.str());
                    }

                    frame_counter++;
                } else {
                    Magnum::Debug{} << "End of stream.";
                    break;
                }
            } catch (k4a::error &e) {
                if (std::string(e.what()) == "Failed to get next capture!") {
                    Magnum::Debug{} << "Playback stopped";
                    break;
                } else {
                    Magnum::Error{} << "Error during playback: " << e.what();
                    return 1;
                }
            }
        }
        m_dev.close();
        Magnum::Debug{} << "Done.";
        return 0;

    }

    void ExtractFramesMKV::save_calibration(k4a_calibration_t calibration) {
        // from Kinect SDK ...

        // converting the calibration data to OpenCV format
        // extrinsic transformation from color to depth camera
        cv::Mat se3 = cv::Mat(3, 3, CV_32FC1,
                              calibration.extrinsics[K4A_CALIBRATION_TYPE_COLOR][K4A_CALIBRATION_TYPE_DEPTH].rotation);
        cv::Mat t_vec = cv::Mat(3, 1, CV_32F,
                                calibration.extrinsics[K4A_CALIBRATION_TYPE_COLOR][K4A_CALIBRATION_TYPE_DEPTH].translation);

        // intrinsic parameters of the depth camera
        k4a_calibration_intrinsic_parameters_t *intrinsics = &calibration.depth_camera_calibration.intrinsics.parameters;
        std::vector<float> _depth_camera_matrix = {
                intrinsics->param.fx, 0.f, intrinsics->param.cx, 0.f, intrinsics->param.fy, intrinsics->param.cy, 0.f,
                0.f, 1.f
        };
        cv::Mat depth_camera_matrix = cv::Mat(3, 3, CV_32F, &_depth_camera_matrix[0]);
        std::vector<float> _depth_dist_coeffs = {intrinsics->param.k1, intrinsics->param.k2, intrinsics->param.p1,
                                                 intrinsics->param.p2, intrinsics->param.k3, intrinsics->param.k4,
                                                 intrinsics->param.k5, intrinsics->param.k6};
        cv::Mat depth_dist_coeffs = cv::Mat(8, 1, CV_32F, &_depth_dist_coeffs[0]);

        // intrinsic parameters of the color camera
        intrinsics = &calibration.color_camera_calibration.intrinsics.parameters;
        std::vector<float> _color_camera_matrix = {
                intrinsics->param.fx, 0.f, intrinsics->param.cx, 0.f, intrinsics->param.fy, intrinsics->param.cy, 0.f,
                0.f, 1.f
        };
        cv::Mat color_camera_matrix = cv::Mat(3, 3, CV_32F, &_color_camera_matrix[0]);
        std::vector<float> _color_dist_coeffs = {intrinsics->param.k1, intrinsics->param.k2, intrinsics->param.p1,
                                                 intrinsics->param.p2, intrinsics->param.k3, intrinsics->param.k4,
                                                 intrinsics->param.k5, intrinsics->param.k6};
        cv::Mat color_dist_coeffs = cv::Mat(8, 1, CV_32F, &_color_dist_coeffs[0]);

        // store configuration in output directory
        std::string config_fname = Corrade::Utility::Directory::join(m_output_directory, "camera_calibration.yml");
        cv::FileStorage cfg_fs(config_fname, cv::FileStorage::WRITE);
        cfg_fs << "depth_image_width" << calibration.depth_camera_calibration.resolution_width;
        cfg_fs << "depth_image_height" << calibration.depth_camera_calibration.resolution_height;
        cfg_fs << "depth_camera_matrix" << depth_camera_matrix;
        cfg_fs << "depth_distortion_coefficients" << depth_dist_coeffs;

        cfg_fs << "color_image_width" << calibration.color_camera_calibration.resolution_width;
        cfg_fs << "color_image_height" << calibration.color_camera_calibration.resolution_height;
        cfg_fs << "color_camera_matrix" << color_camera_matrix;
        cfg_fs << "color_distortion_coefficients" << color_dist_coeffs;

        cfg_fs << "depth2color_translation" << t_vec;
        cfg_fs << "depth2color_rotation" << se3;
    }

    void ExtractFramesMKV::process_depth(k4a::capture capture, int frame_counter) {
        const k4a::image inputDepthImage = capture.get_depth_image();
        if (inputDepthImage) {
            // record depth timestamp
            m_tsss << std::to_string(inputDepthImage.get_device_timestamp().count()) << ",";
            m_tsss << std::to_string(inputDepthImage.get_system_timestamp().count()) << ",";
            if (m_export_depth) {

                int w = inputDepthImage.get_width_pixels();
                int h = inputDepthImage.get_height_pixels();

                if (inputDepthImage.get_format() == K4A_IMAGE_FORMAT_DEPTH16) {
                    cv::Mat image_buffer = cv::Mat(cv::Size(w, h), CV_16UC1,
                                                   const_cast<void *>(static_cast<const void *>(inputDepthImage.get_buffer())),
                                                   static_cast<size_t>(inputDepthImage.get_stride_bytes()));
                    uint64_t timestamp = inputDepthImage.get_system_timestamp().count();

                    std::ostringstream ss;
                    ss << std::setw(10) << std::setfill('0') << frame_counter << "_depth.tiff";
                    std::string image_path = Corrade::Utility::Directory::join(m_output_directory, ss.str());
                    cv::imwrite(image_path, image_buffer);

                } else {
                    Magnum::Warning{} << "Received depth frame with unexpected format: "
                                      << inputDepthImage.get_format();
                    throw MissingDataException();
                }
            }
        } else {
            m_tsss << ",,";
        }
    }

    void ExtractFramesMKV::process_color(k4a::capture capture, int frame_counter) {
        const k4a::image inputColorImage = capture.get_color_image();
        {
            if (inputColorImage) {
                // record color timestamp
                m_tsss << std::to_string(inputColorImage.get_device_timestamp().count()) << ",";
                m_tsss << std::to_string(inputColorImage.get_system_timestamp().count()) << ",";

                if (m_export_color) {
                    int w = inputColorImage.get_width_pixels();
                    int h = inputColorImage.get_height_pixels();

                    cv::Mat image_buffer;
                    uint64_t timestamp;

                    if (inputColorImage.get_format() == K4A_IMAGE_FORMAT_COLOR_BGRA32) {
                        image_buffer = cv::Mat(cv::Size(w, h), CV_8UC4,
                                               const_cast<void *>(static_cast<const void *>(inputColorImage.get_buffer())),
                                               cv::Mat::AUTO_STEP);
                        timestamp = inputColorImage.get_system_timestamp().count();

                        std::vector<int> compression_params;
                        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
                        compression_params.push_back(95);

                        std::ostringstream ss;
                        ss << std::setw(10) << std::setfill('0') << frame_counter << "_color.jpg";
                        std::string image_path = Corrade::Utility::Directory::join(m_output_directory, ss.str());
                        cv::imwrite(image_path, image_buffer, compression_params);

                    } else if (inputColorImage.get_format() == K4A_IMAGE_FORMAT_COLOR_MJPG) {

                        auto rawData = Corrade::Containers::ArrayView<uint8_t>(const_cast<uint8_t *>(inputColorImage.get_buffer()), inputColorImage.get_size());
                        std::ostringstream ss;
                        ss << std::setw(10) << std::setfill('0') << frame_counter << "_color.jpg";
                        std::string image_path = Corrade::Utility::Directory::join(m_output_directory, ss.str());
                        Corrade::Utility::Directory::write(image_path, rawData);

                    } else {
                        Magnum::Warning{} << "Received color frame with unexpected format: "
                                          << inputColorImage.get_format();
                        throw MissingDataException();
                    }

                }
            } else {
                m_tsss << ",,";
            }
        }
    }
    void ExtractFramesMKV::process_ir(k4a::capture capture, int frame_counter) {
        const k4a::image inputIRImage = capture.get_ir_image();
        {
            if (inputIRImage) {
                // record infrared timestamp
                m_tsss << std::to_string(inputIRImage.get_device_timestamp().count()) << ",";
                m_tsss << std::to_string(inputIRImage.get_system_timestamp().count()) << "\n";

                if (m_export_infrared) {
                    int w = inputIRImage.get_width_pixels();
                    int h = inputIRImage.get_height_pixels();

                    if (inputIRImage.get_format() == K4A_IMAGE_FORMAT_IR16) {
                        cv::Mat image_buffer = cv::Mat(cv::Size(w, h), CV_16UC1,
                                                       const_cast<void *>(static_cast<const void *>(inputIRImage.get_buffer())),
                                                       static_cast<size_t>(inputIRImage.get_stride_bytes()));
                        uint64_t timestamp = inputIRImage.get_system_timestamp().count();

                        std::ostringstream ss;
                        ss << std::setw(10) << std::setfill('0') << frame_counter << "_ir.tiff";
                        std::string image_path = Corrade::Utility::Directory::join(m_output_directory, ss.str());
                        cv::imwrite(image_path, image_buffer);

                    } else {
                        Magnum::Warning{} << "Received infrared frame with unexpected format: "
                                          << inputIRImage.get_format();
                        throw MissingDataException();
                    }
                }
            } else {
                m_tsss << ",\n";
            }
        }
    }

    void ExtractFramesMKV::process_rgbd(k4a::capture capture, int frame_counter, k4a_calibration_t calibration) {

        const k4a::image inputDepthImage = capture.get_depth_image();
        const k4a::image inputColorImage = capture.get_color_image();

        int color_image_width_pixels = k4a_image_get_width_pixels(inputColorImage.handle());
        int color_image_height_pixels = k4a_image_get_height_pixels(inputColorImage.handle());

        if (m_export_rgbd) {
            if (!(inputColorImage && inputDepthImage)) {
                Magnum::Warning{} << "Export RGBD requires depth and color image.";
                throw MissingDataException();
            }
            k4a_image_t transformed_depth_image;
            if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,
                                                         color_image_width_pixels,
                                                         color_image_height_pixels,
                                                         color_image_width_pixels * (int)sizeof(uint16_t),
                                                         &transformed_depth_image))
            {
                Magnum::Error{} << "Failed to create transformed color image";
                throw MissingDataException();
            }

            k4a_transformation_t transformation = k4a_transformation_create(&calibration);
            if (K4A_RESULT_SUCCEEDED !=
                    k4a_transformation_depth_image_to_color_camera(transformation, inputDepthImage.handle(),
                                                                   transformed_depth_image))
            {
                Magnum::Error{} << "Failed to compute transformed depth image";
                throw MissingDataException();
            }
            std::ostringstream ss;
            ss << std::setw(10) << std::setfill('0') << frame_counter << "_rgbd.tiff";
            std::string image_path = Corrade::Utility::Directory::join(m_output_directory, ss.str());
            cv::Mat image_buffer = cv::Mat(cv::Size(color_image_width_pixels, color_image_height_pixels), CV_16UC1,
                                           const_cast<void *>(static_cast<const void *>(k4a_image_get_buffer(transformed_depth_image))),
                                           static_cast<size_t>(k4a_image_get_stride_bytes(transformed_depth_image)));

            cv::imwrite(image_path, image_buffer);
            k4a_image_release(transformed_depth_image);
            k4a_transformation_destroy(transformation);
        }
    }
//
//                    if (m_export_pointcloud) {
//
//                        k4a_transformation_t transformation = k4a_transformation_create(&calibration);
//                        // transform color image into depth camera geometry
//                        int depth_image_width_pixels = k4a_image_get_width_pixels(capture.get_depth_image().handle());
//                        int depth_image_height_pixels = k4a_image_get_height_pixels(capture.get_depth_image().handle());
//                        k4a_image_t transformed_color_image = NULL;
//                        k4a::image color_image;
//                        cv::Mat result;
//
//
//                        if (capture.get_color_image().get_format() == K4A_IMAGE_FORMAT_COLOR_BGRA32) {
//                            color_image = capture.get_color_image();
//
//                        } else if (capture.get_color_image().get_format() == K4A_IMAGE_FORMAT_COLOR_MJPG) {
//
//                            cv::Mat rawData(1, capture.get_color_image().get_size(), CV_8SC1,
//                                            const_cast<void *>(static_cast<const void *>(capture.get_color_image().get_buffer())));
//                            cv::Mat image_buffer = cv::imdecode(rawData, -cv::IMREAD_COLOR);
//
//                            cv::cvtColor(image_buffer, result, cv::COLOR_BGR2BGRA);
//                            color_image = k4a::image::create_from_buffer(K4A_IMAGE_FORMAT_COLOR_BGRA32,
//                                                                         color_image_width_pixels,
//                                                                         color_image_height_pixels,
//                                                                         color_image_width_pixels * 4 * (int)sizeof(unsigned char),
//                                                                         result.data, result.total() * result.elemSize(), NULL, NULL);
//
//                        } else {
//                            Magnum::Warning{} << "Received color frame with unexpected format: "
//                                              << capture.get_color_image().get_format();
//                            break;
//                        }
//
//
//
//                        if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_COLOR_BGRA32,
//                                                                     depth_image_width_pixels,
//                                                                     depth_image_height_pixels,
//                                                                     depth_image_width_pixels * 4 * (int)sizeof(uint8_t),
//                                                                     &transformed_color_image))
//                        {
//                            Magnum::Error{} << "Failed to create transformed color image";
//                            break;
//                        }
//
//                        k4a_image_t point_cloud_image = NULL;
//                        if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
//                                                                     depth_image_width_pixels,
//                                                                     depth_image_height_pixels,
//                                                                     depth_image_width_pixels * 3 * (int)sizeof(int16_t),
//                                                                     &point_cloud_image))
//                        {
//                            Magnum::Error{} << "Failed to create point cloud image";
//                            break;
//                        }
//
//                        if (K4A_RESULT_SUCCEEDED !=
//                                k4a_transformation_color_image_to_depth_camera(transformation, capture.get_depth_image().handle(), color_image.handle(), transformed_color_image))
//                        {
//                            Magnum::Error{} << "Failed to compute transformed color image";
//                            break;
//                        }
//
//                        if (K4A_RESULT_SUCCEEDED != k4a_transformation_depth_image_to_point_cloud(transformation,
//                                                                                                  capture.get_depth_image().handle(),
//                                                                                                  K4A_CALIBRATION_TYPE_DEPTH,
//                                                                                                  point_cloud_image))
//                        {
//                            Magnum::Error{} << "Failed to compute point cloud";
//                            break;
//                        }
//
//                        std::ostringstream ss;
//                        ss << std::setw(10) << std::setfill('0') << frame_counter << "_pointcloud.ply";
//                        std::string ply_path = Corrade::Utility::Directory::join(m_output_directory, ss.str());
//
//                        tranformation_helpers_write_point_cloud(point_cloud_image, transformed_color_image, ply_path.c_str());
//
//                        k4a_image_release(transformed_color_image);
//                        k4a_image_release(point_cloud_image);
//
//                    }
//
//    }

}

MAGNUM_WINDOWLESSAPPLICATION_MAIN(Magnum::ExtractFramesMKV)

