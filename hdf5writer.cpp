/**
 * jadaq (Just Another DAQ)
 * Copyright (C) 2017  Troels Blum <troels@blum.dk>
 * Contributions from  Jonas Bardino <bardino@nbi.ku.dk>
 *
 * @file
 * @author Troels Blum <troels@blum.dk>
 * @section LICENSE
 * This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *         but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * A simple daemon listening for data and writing it to HDF5 file.
 *
 */

#include <signal.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>

#include "H5Cpp.h"
using namespace H5;


#include "DataFormat.hpp"


/* Keep running marker and interrupt signal handler */
static int interrupted = 0;
static void interrupt_handler(int s)
{
    interrupted = 1;
}
static void setup_interrupt_handler()
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = interrupt_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);
}

int main(int argc, char **argv) {
    if (argc > 2)
    {
        std::cout << "Usage: " << argv[0] << " [<config_file>] [<output_file>]" << std::endl;
        std::cout << "Reads in a partial/full configuration in <config_file> " << std::endl;
        std::cout << "and configures the hdf5writer accordingly. Then dumps " << std::endl;
        std::cout << "received data as HDF5 into <output_file>. " << std::endl;
        return -1;
    }

    /* Helpers */
    uint32_t eventsWritten = 0;

    uint32_t eventIndex = 0, i = 0, j = 0;
    /* Dummy data */
    std::string digitizer, digitizerModel, digitizerID;
    uint32_t channel = 0, charge = 0 , globaltime = 0, localtime = 0;
    
    /* Path helpers */
    std::string configFileName;
    std::string outputFileName = "out.h5";

    const H5std_string FILE_NAME(outputFileName);
    
    /* Act on command-line arguments */
    if (argc > 1) {
        configFileName = std::string(argv[1]);
        std::cout << "Reading hdf5writer configuration from: " << configFileName << std::endl;
    } else {
        std::cout << "Using default hdf5writer configuration." << std::endl;
    }
    if (argc > 2) {
        outputFileName = std::string(argv[2]);
        std::cout << "Writing hdf5 formatted data to: " << outputFileName << std::endl;
    } else {
        std::cout << "Using default hdf5 output location: " << outputFileName << std::endl;
    }

    /* Prepare and start event handling */
    std::cout << "Setup hdf5writer" << std::endl;

    /* TODO: setup UDP listener */
    
    /* Prepare output file  and data sets */
    bool createOutfile = true, closeOutfile = false;
    bool createGroup = true, closeGroup = false;
    bool createDataset = true, closeDataset = false;
    H5std_string GROUP_NAME, DATASET_NAME;
    std::stringstream nest;
    H5File outfile;
    Group group;
    DataSet dataset;
    try {
        /* TODO: should we truncate or just keep adding? */
        if (createOutfile) {
            std::cout << "Creating new outfile " << FILE_NAME << std::endl;
            outfile = H5File(FILE_NAME, H5F_ACC_TRUNC);
        } else {    
            std::cout << "Opening existing outfile " << FILE_NAME << std::endl;
            outfile = H5File(FILE_NAME, H5F_ACC_RDWR);
        }
    } catch(FileIException error) {
        std::cerr << "ERROR: could not open/create outfile " << FILE_NAME << std::endl;
        error.printError();
        exit(1);
    }
    
    /* TODO: switch to real data format */
    uint32_t data[4];
    const int RANK = 1;
    hsize_t dimsf[4];
    dimsf[0] = 4;
    DataSpace dataspace(RANK, dimsf);
    IntType datatype(PredType::NATIVE_INT);
    datatype.setOrder(H5T_ORDER_LE);

    /* Turn off the auto-printing of errors - manual print instead. */
    Exception::dontPrint();

    /* Set up interrupt handler and start handling acquired data */
    setup_interrupt_handler();

    std::cout << "Running file writer loop - Ctrl-C to interrupt" << std::endl;

    uint32_t throttleDown = 100;
    bool keepRunning = true;
    while(keepRunning) {
        /* Continuously receive and dump data */
        if (throttleDown) {
            /* NOTE: for running without hogging CPU if nothing to do */
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            throttleDown = 100;
        }
        try {
            std::cout << "Receive data" << std::endl;
            /* TODO: actualy receive data here - fake for now */
            digitizerModel = "V1740D";
            digitizerID = "137";
            digitizer = "V1740D_137";
            channel = 31;
            charge = 42;
            globaltime = std::time(nullptr);
            localtime = globaltime & 0xFFFF;
            std::cout << "Saving data from " << digitizer << " channel " << channel << " localtime " << localtime << " globaltime " << globaltime << " charge " << charge << std::endl;

            
            /* Create a new group for the digitizer if it doesn't
             * exist in the output file. */
            GROUP_NAME = H5std_string(digitizer);
            createGroup = true;
            try {
                std::cout << "Try to open group " << GROUP_NAME << std::endl;
                group = outfile.openGroup(GROUP_NAME);
                createGroup = false;
            } catch(FileIException error) {
                if (!createOutfile) {
                    std::cerr << "ERROR: could not open group " << GROUP_NAME << " : " << std::endl;
                    error.printError();
                } else {
                    //std::cout << "DEBUG: could not open group " << GROUP_NAME << " : " << std::endl;
                }
            }
            if (createGroup) {
                std::cout << "Create group " << GROUP_NAME << std::endl;
                group = outfile.createGroup(GROUP_NAME);
                createGroup = false;
            }
            //closeGroup = true;

            /* Create a new dataset for globaltime stamp if it doesn't
             * exist in the group of the output file. */
            nest.str("");
            nest.clear();
            nest << GROUP_NAME << "/" << globaltime;
            DATASET_NAME = H5std_string(nest.str());
            createDataset = true;
            try {
                std::cout << "Try to open dataset " << DATASET_NAME << std::endl;
                dataset = outfile.openDataSet(DATASET_NAME);
                createDataset = false;
            } catch(FileIException error) {
                if (!createOutfile) {
                    std::cerr << "ERROR: could not open dataset " << DATASET_NAME << " : file exception : " << std::endl;
                    error.printError();
                } else {
                    //std::cout << "DEBUG: could not open dataset " << DATASET_NAME << " : " << std::endl;
                }
            }
            if (createDataset) {
                std::cout << "Create dataset " << DATASET_NAME << std::endl;
                dataset = outfile.createDataSet(DATASET_NAME, datatype, dataspace);
                createDataset = false;
            }
            closeDataset = true;
            
            data[0] = channel;
            data[1] = globaltime;
            data[2] = localtime;
            data[3] = charge;
            dataset.write(data, PredType::NATIVE_INT);

            dataset.close();
            closeDataset = false;
            /* TODO: close needed? */
            /*
            group.close;
            closeGroup = false;
            */
            
        } catch(std::exception& e) {
            std::cerr << "unexpected exception during reception: " << e.what() << std::endl;
            /* NOTE: throttle down on errors */
            throttleDown = 2000;
        }
        if (interrupted) {
            std::cout << "caught interrupt - stop file writer and clean up." << std::endl;
            break;
        }
    }

    /* Stop file writer */
    std::cout << "Stop file writer" << std::endl;

    /* TODO: close UDP listener */
    /* TODO: close data sets and output file */
    if (closeDataset) {
        dataset.close();
    }
    if (closeGroup) {
        group.close();
    }
    if (closeOutfile) {
        outfile.close();
    }
    
    /* Clean up after all */
    std::cout << "Clean up after file writer" << std::endl;


    std::cout << "Shutting down." << std::endl;

    return 0;
}