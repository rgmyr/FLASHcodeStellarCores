#ifndef HDFIO_H
#define HDFIO_H

#include <vector>
#include <string>
#include <iostream>
#include <hdf5.h>
#include <assert.h>

/**
 * HDFIO class
 *
 * This class represents an HDF5 object/file and provides basic
 * HDF operations like opening a file, reading data form a dataset
 * of that file, creating new HDF files and datasets and writing
 * datasets into a created file. There are also some functions
 * for getting and setting different attributes, like the
 * dimensionaltity of the HDF dataset to be written or the
 * datasetnames inside an HDF5 file.
 *
 * @author Christoph Federrath
 * @version 2007-2012
 *
 * @author Ross Meyer
 * @updated: June 2014 
 *      The updated HDF5 library maps H5Dopen --> H5Dopen2, instead of H5Dopen1,
 *      and " for H5Dcreate. 
 *      You can probably use H5P_DEFAULT for the extra function arguments -- or take
 *      a look at the HDF5 API specification. Uncomment the extra args where necessary.
 */

class HDFIO
{
	private:
		int Rank;			//dimensionality of the field

		// hdf 5 stuff
        hsize_t HDFSize, HDFDims[4];
		hid_t   File_id, Dataset_id, Dataspace_id;
		herr_t  HDF5_status, HDF5_error;

        std::string Filename;

	public:
		
		/**
		 * Default constructor.
		 *
		 */
		HDFIO()
		: Rank(1),				//dimensionality 1 (x only)
          HDFSize(0), // set buffer size to 0
		  File_id(0),				//set all to 0
		  Dataset_id(0),
		  Dataspace_id(0),
		  HDF5_status(0),			//set HDF5 status to 0
		  HDF5_error(-1),			//set to -1
          Filename()				//create Nullstring
		{
			for(int i = 0; i < 4; i++)
				HDFDims[i] = 0;         //set Dimensions to (0,0,0,0)
		}


		/**
		 * open an HDF5 file
		 * @param Filename HDF5 filename
		 * @param read_write_char 'r': read only flag, 'w': write flag
		 *
		 */
		void open(const std::string Filename, const char read_write_char)
		{
			this->Filename = Filename;

			switch (read_write_char)
			{
				case 'r':
				{
					// open HDF5 file in read only mode
					File_id = H5Fopen(Filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
					assert( File_id != HDF5_error );
					break;
				}
				case 'w':
				{
					// open HDF5 file in write mode
					File_id = H5Fopen(Filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
					assert( File_id != HDF5_error );
					break;
				}
				default:
				{
					// open HDF5 file in read only mode
					File_id = H5Fopen(Filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
					assert( File_id != HDF5_error );
					break;
				}
			}
		};

		/**
		 * close HDF5 file
		 *
		 */
		void close(void)
		{
			// close HDF5 file
			HDF5_status = H5Fclose(File_id);
            assert( HDF5_status != HDF5_error );
        }

		/**
		 * read data from a dataset
		 * @param *DataBuffer pointer to double/float/int array to which data is to be written
		 * @param Datasetname datasetname
		 * @param DataType (i.e. H5T_STD_I32LE)
		 *
		 */
		void read(void* const DataBuffer, const std::string Datasetname, const hid_t DataType)
		{
			// get dimensional information from dataspace and update HDFSize
			getDims(Datasetname);

			// open dataset
			Dataset_id = H5Dopen(File_id, Datasetname.c_str());// H5P_DEFAULT);
			assert( Dataset_id != HDF5_error );

			// open dataspace (to get dimensions)
			Dataspace_id = H5Dget_space(Dataset_id);
			assert( Dataspace_id != HDF5_error );

			// read buffer                               //memspaceid //filespaceid
			HDF5_status = H5Dread( Dataset_id, DataType, H5S_ALL, H5S_ALL,
						H5P_DEFAULT, DataBuffer );
			assert( HDF5_status != HDF5_error );

			HDF5_status = H5Sclose(Dataspace_id);
			assert( HDF5_status != HDF5_error );

			HDF5_status = H5Dclose(Dataset_id);
			assert( HDF5_status != HDF5_error );

		}//read

		/**
		 * overwrite data of an existing dataset
		 * @param *DataBuffer pointer to double/float/int array containing data to be written
		 * @param Datasetname datasetname
		 * @param DataType (i.e. H5T_STD_I32LE)
		 *
		 */
		void overwrite(const void* const DataBuffer, const std::string Datasetname, const hid_t DataType)
		{
			// get dimensional information from dataspace and update HDFSize
			getDims(Datasetname);

			// open dataset
			Dataset_id = H5Dopen(File_id, Datasetname.c_str());// H5P_DEFAULT);
			assert( Dataset_id != HDF5_error );

			// open dataspace (to get dimensions)
			Dataspace_id = H5Dget_space(Dataset_id);
			assert( Dataspace_id != HDF5_error );

			// overwrite dataset
			HDF5_status = H5Dwrite( Dataset_id, DataType,
					H5S_ALL, H5S_ALL, H5P_DEFAULT, DataBuffer);
			assert( HDF5_status != HDF5_error ); 

			HDF5_status = H5Sclose(Dataspace_id);
			assert( HDF5_status != HDF5_error );

			HDF5_status = H5Dclose(Dataset_id);
			assert( HDF5_status != HDF5_error );

		}//overwrite


		/**
		 * create new HDF5 file
		 * @param Filename HDF5 filename
		 *
		 */
		void create(const std::string Filename)
		{
			this->Filename = Filename;

			// create HDF5 file
			File_id = H5Fcreate(Filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
			assert( File_id != HDF5_error );
		}


		/**
		 * write HDF5 dataset
		 * @param DataBuffer int/float/double array containing the data
		 * @param Datasetname datasetname
		 * @param Dimensions dataset dimensions
		 * @param DataType (i.e. H5T_STD_I32LE)
		 *
		 */
		void write(const void* const DataBuffer, const std::string Datasetname,
				const std::vector<int> Dimensions, const hid_t DataType)
		{
			setDims(Dimensions);				// set dimensions
			write(DataBuffer, Datasetname, DataType);	// call write()
		}


		/**
		 * write HDF5 dataset
		 * @param *DataBuffer pointer to int/float/double array containing the data
		 * @param Datasetname datasetname
		 * @param DataType (i.e. H5T_IEEE_F32BE, H5T_STD_I32LE, ...)
		 *
		 */
		void write(const void* const DataBuffer, const std::string Datasetname, const hid_t DataType)
		{
			// -------------- create dataspace
			Dataspace_id = H5Screate_simple(Rank, HDFDims, NULL);
			assert( Dataspace_id != HDF5_error );

			// -------------- create dataset
			Dataset_id = H5Dcreate(File_id, Datasetname.c_str(),
						DataType, Dataspace_id, H5P_DEFAULT);// H5P_DEFAULT, H5P_DEFAULT );
			assert( Dataset_id != HDF5_error );

			// -------------- write dataset
			HDF5_status = H5Dwrite(Dataset_id, DataType,
					H5S_ALL, H5S_ALL, H5P_DEFAULT, DataBuffer);
			assert( HDF5_status != HDF5_error );

			// -------------- close dataset
			HDF5_status = H5Dclose(Dataset_id);
			assert( HDF5_status != HDF5_error );

			// -------------- close dataspace
			HDF5_status = H5Sclose(Dataspace_id);
			assert( HDF5_status != HDF5_error );
		}


		/**
		 * set the rank of a dataset
		 * @param Rank the rank, dimensionality (0,1,2)
		 *
		 */
		void setRank(const int Rank)
		{
			this->Rank = Rank;
		}


		/**
		 * set array dimension in different directions
		 * @param dim dimension of the array
		 *
		 */
		void setDimX(const int dim_x)
		{
			HDFDims[0] = static_cast<hsize_t>(dim_x);
		}

		void setDimY(const int dim_y)
		{
			HDFDims[1] = static_cast<hsize_t>(dim_y);
		}

		void setDimZ(const int dim_z)
		{
			HDFDims[2] = static_cast<hsize_t>(dim_z);
		}

		void setDims(const std::vector<int> Dimensions)
		{
			Rank = Dimensions.size();
			for(int i = 0; i < Rank; i++)
				HDFDims[i] = static_cast<hsize_t>(Dimensions[i]);
		}


		/**
		 * get the rank of the current dataset
		 * @return int dimensionality
		 *
		 */
		int getRank(void) const
		{
			return Rank;
		}


		/**
		 * get the rank of a dataset with name datasetname
		 * @param Datasetname datasetname
		 * @return int dimensionality
		 *
		 */
		int getRank(const std::string Datasetname)
		{
			// get dimensional information from dataspace and update HDFSize
			getDims(Datasetname);
			return Rank;
		}


		/**
		 * get array dimension in different directions
		 * @return dimension of the array
		 *
		 */
		int getDimX(void) const
		{
			return static_cast<int>(HDFDims[0]);
		}

		int getDimY(void) const
		{
			return static_cast<int>(HDFDims[1]);
		}

		int getDimZ(void) const
		{
			return static_cast<int>(HDFDims[2]);
		}

		/**
		 * get dataset size of dataset with datasetname
		 * @param Datasetname datasetname
		 * @return size of the dataset
		 *
		 */
		int getSize(const std::string Datasetname)
		{
			// open dataset
			Dataset_id = H5Dopen(File_id, Datasetname.c_str());// H5P_DEFAULT);
			if (Dataset_id == HDF5_error)
			{
				std::cout << "HDFIO:  getSize():  CAUTION: Datasetname '" << Datasetname << "' does not exists in file '"<<this->getFilename()<<"'. Continuing..." << std::endl;
				return 0;
			}
			assert( Dataset_id != HDF5_error );

			// open dataspace
			Dataspace_id = H5Dget_space(Dataset_id);
			assert( Dataspace_id != HDF5_error );

			// get dimensional information from dataspace
			hsize_t HDFxdims[4], HDFmaxdims[4];
			Rank = H5Sget_simple_extent_dims(Dataspace_id, HDFxdims, HDFmaxdims);

			// from the dimensional info, calculate the size of the buffer.
			HDFSize = 1;
			for (int i = 0; i < Rank; i++) {
				HDFDims[i] = HDFxdims[i];
				HDFSize *= HDFDims[i];
			}

			HDF5_status = H5Sclose(Dataspace_id);
			assert( HDF5_status != HDF5_error );

			HDF5_status = H5Dclose(Dataset_id);
			assert( HDF5_status != HDF5_error );

			return static_cast<int>(HDFSize);
		}

		/**
		 * get array dimension in different directions and update HDFSize
		 * @param Datasetname datasetname for which dimensional info is read
		 * @return dimension of the array
		 *
		 */
		std::vector<int> getDims(const std::string Datasetname)
		{
			// open dataset
			Dataset_id = H5Dopen(File_id, Datasetname.c_str());// H5P_DEFAULT);
			assert( Dataset_id != HDF5_error );

			// open dataspace
			Dataspace_id = H5Dget_space(Dataset_id);
			assert( Dataspace_id != HDF5_error );

			// get dimensional information from dataspace
			hsize_t HDFxdims[4], HDFmaxdims[4];

			Rank = H5Sget_simple_extent_dims(Dataspace_id, HDFxdims, HDFmaxdims);

			// from the dimensional info, calculate the size of the buffer.
			HDFSize = 1;
			for (int i = 0; i < Rank; i++) {
				HDFDims[i] = HDFxdims[i];
				HDFSize *= HDFDims[i];
			}

			HDF5_status = H5Sclose(Dataspace_id);
			assert( HDF5_status != HDF5_error );

			HDF5_status = H5Dclose(Dataset_id);
			assert( HDF5_status != HDF5_error );

			std::vector<int> ReturnDims(Rank);
			for(int i = 0; i < Rank; i++)
				ReturnDims[i] = static_cast<int>(HDFDims[i]);

			return ReturnDims;
		}

		/**
		 * get HDF5 filename
		 * @return filename
		 *
		 */
		std::string getFilename(void) const
		{
			return Filename;
		}

		/**
		 * get number of datasets in HDF5 file
		 * @return the number of datasets
		 *
		 */
		int getNumberOfDatasets(void) const
		{
			hsize_t *NumberofObjects = new hsize_t[1];
			H5Gget_num_objs(File_id, NumberofObjects);
			int returnNumber = static_cast<int>(NumberofObjects[0]);
			delete [] NumberofObjects;
			return returnNumber;
		}

		/**
		 * get HDF5 datasetname
		 * @param datasetnumber integer number identifying the dataset
		 * @return datasetname
		 *
		 */
		std::string getDatasetname(const int datasetnumber) const
		{
			char *Name = new char[256];
			H5Gget_objname_by_idx(File_id, datasetnumber, Name, 256);
			std::string returnName = static_cast<std::string>(Name);
			delete [] Name;
			return returnName;
		}

}; // end: HDFIO
#endif
