/*
 * Endian.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include "../ul_internal.h"

namespace ul
{

class UL_LOCAL Endian
{
public:
	static Endian& Instance()
	{
		static Endian mInstance;
		return mInstance;
	}

	static inline unsigned short cpu_to_le_ui16(const unsigned short x)
	{
		union
		{
			unsigned char  b8[2];
			unsigned short b16;
		} _tmp;
		_tmp.b8[1] = (unsigned char) (x >> 8);
		_tmp.b8[0] = (unsigned char) (x & 0xff);
		return _tmp.b16;
	}

	#define le_ui16_to_cpu cpu_to_le_ui16

	static inline short cpu_to_le_i16(const short x)
	{
		union
		{
			unsigned char  b8[2];
			short b16;
		} _tmp;
		_tmp.b8[1] = (unsigned char) (x >> 8);
		_tmp.b8[0] = (unsigned char) (x & 0xff);
		return _tmp.b16;
	}

	#define le_i16_to_cpu cpu_to_le_i16

	static inline unsigned int cpu_to_le_ui32(const unsigned int x)
	{
		union
		{
			unsigned char  b8[4];
			unsigned int b32;
		} _tmp;
		_tmp.b8[3] = (unsigned char) (x >> 24);
		_tmp.b8[2] = (unsigned char) (x >> 16 & 0xff);
		_tmp.b8[1] = (unsigned char) (x >> 8 & 0xff);
		_tmp.b8[0] = (unsigned char) (x & 0xff);
		return _tmp.b32;
	}

	#define le_ui32_to_cpu cpu_to_le_ui32

	static inline unsigned long long cpu_to_le_ui64(const unsigned long long x)
	{
		union
		{
			unsigned char  b8[8];
			unsigned long long b64;
		} _tmp;

		_tmp.b8[7] = (unsigned char) (x >> 56);
		_tmp.b8[6] = (unsigned char) (x >> 48 & 0xff);
		_tmp.b8[5] = (unsigned char) (x >> 40 & 0xff);
		_tmp.b8[4] = (unsigned char) (x >> 32 & 0xff);
		_tmp.b8[3] = (unsigned char) (x >> 24 & 0xff);
		_tmp.b8[2] = (unsigned char) (x >> 16 & 0xff);
		_tmp.b8[1] = (unsigned char) (x >> 8 & 0xff);
		_tmp.b8[0] = (unsigned char) (x & 0xff);
		return _tmp.b64;
	}

	#define le_ui64_to_cpu cpu_to_le_ui64


	static inline int isLittleEndian()
	{
		unsigned short i = 1;
		char *ptr = (char *) &i;
		return  (*ptr);
	}

	inline short le_ptr_to_cpu_i16(const unsigned char* x)
	{
		union
		{
			unsigned char  b8[2];
			short b16;
		} _tmp;

		if(mLittleEndian)
			_tmp.b16 = *((short*)x);
		else
		{
			_tmp.b8[1] = x[0];
			_tmp.b8[0] = x[1];
		}
		return _tmp.b16;
	}

	inline int le_ptr_to_cpu_i32(const unsigned char x[4])
	{
		union
		{
			unsigned char  b8[4];
			int b32;
		} _tmp;

		if(mLittleEndian)
			_tmp.b32 = *((int*)x);
		else
		{
			_tmp.b8[3] = x[0];
			_tmp.b8[2] = x[1];
			_tmp.b8[1] = x[2];
			_tmp.b8[0] = x[3];
		}
		return _tmp.b32;
	}

	inline unsigned int le_ptr_to_cpu_ui32(const unsigned char x[4])
	{
		union
		{
			unsigned char  b8[4];
			unsigned int b32;
		} _tmp;

		if(mLittleEndian)
			_tmp.b32 = *((int*)x);
		else
		{
			_tmp.b8[3] = x[0];
			_tmp.b8[2] = x[1];
			_tmp.b8[1] = x[2];
			_tmp.b8[0] = x[3];
		}
		return _tmp.b32;
	}

	inline int be_ptr_to_cpu_i32(const unsigned char x[4])
	{
		union
		{
			unsigned char  b8[4];
			int b32;
		} _tmp;

		if(!mLittleEndian)
			_tmp.b32 = *((int*)x);
		else
		{
			_tmp.b8[3] = x[0];
			_tmp.b8[2] = x[1];
			_tmp.b8[1] = x[2];
			_tmp.b8[0] = x[3];
		}
		return _tmp.b32;
	}
	inline unsigned int be_ptr_to_cpu_ui32(const unsigned char x[4])
	{
		union
		{
			unsigned char  b8[4];
			unsigned int b32;
		} _tmp;

		if(!mLittleEndian)
			_tmp.b32 = *((unsigned int*)x);
		else
		{
			_tmp.b8[3] = x[0];
			_tmp.b8[2] = x[1];
			_tmp.b8[1] = x[2];
			_tmp.b8[0] = x[3];
		}
		return _tmp.b32;
	}

	inline float le_ptr_to_cpu_f32(const unsigned char x[4])
	{
		union
		{
			unsigned char  b8[4];
			float b32;
		} _tmp;

		if(mLittleEndian)
			_tmp.b32 = *((float*)x);
		else
		{
			_tmp.b8[3] = x[0];
			_tmp.b8[2] = x[1];
			_tmp.b8[1] = x[2];
			_tmp.b8[0] = x[3];
		}
		return _tmp.b32;
	}

	inline void cpu_f32_to_le_ptr(float f32, unsigned char x[4])
	{
		union
		{
			unsigned char  b8[4];
			float b32;
		} _tmp;

		if(mLittleEndian)
			std::memcpy((void*)&f32, (void*)x, sizeof(float));
		else
		{
			_tmp.b32 = f32;
			 x[0] = _tmp.b8[3];
			 x[1] = _tmp.b8[2];
			 x[2] = _tmp.b8[1];
			 x[3] = _tmp.b8[0];
		}
	}

	inline unsigned long long be_ptr_to_cpu_u64(const unsigned char x[8])
	{
		union
		{
			unsigned char  b8[8];
			unsigned long long b64;
		} _tmp;

		if(!mLittleEndian)
			_tmp.b64 = *((unsigned long long*)x);
		else
		{
			_tmp.b8[7] = x[0];
			_tmp.b8[6] = x[1];
			_tmp.b8[5] = x[2];
			_tmp.b8[4] = x[3];
			_tmp.b8[3] = x[4];
			_tmp.b8[2] = x[5];
			_tmp.b8[1] = x[6];
			_tmp.b8[0] = x[7];
		}
		return _tmp.b64;
	}

	static inline unsigned short cpu_to_be_ui16(const unsigned short x)
	{
		union
		{
			unsigned char  b8[2];
			unsigned short b16;
		} _tmp;
		_tmp.b8[0] = (unsigned char) (x >> 8);
		_tmp.b8[1] = (unsigned char) (x & 0xff);
		return _tmp.b16;
	}

	#define be_ui16_to_cpu cpu_to_be_ui16

	inline double le_ptr_to_cpu_f64(const unsigned char x[8])
	{
		union
		{
			unsigned char  b8[8];
			double b64;
		} _tmp;

		if(mLittleEndian)
			_tmp.b64 = *((double*)x);
		else
		{
			_tmp.b8[7] = x[0];
			_tmp.b8[6] = x[1];
			_tmp.b8[5] = x[2];
			_tmp.b8[4] = x[3];
			_tmp.b8[3] = x[4];
			_tmp.b8[2] = x[5];
			_tmp.b8[1] = x[6];
			_tmp.b8[0] = x[7];
		}
		return _tmp.b64;
	}

	inline void cpu_f64_to_le_ptr(double f64, unsigned char x[8])
	{
		union
		{
			unsigned char  b8[8];
			double b64;
		} _tmp;

		if(mLittleEndian)
			std::memcpy((void*)&f64, (void*)x, sizeof(double));
		else
		{
			_tmp.b64 = f64;
			 x[0] = _tmp.b8[7];
			 x[1] = _tmp.b8[6];
			 x[2] = _tmp.b8[5];
			 x[3] = _tmp.b8[4];
			 x[4] = _tmp.b8[3];
			 x[5] = _tmp.b8[2];
			 x[6] = _tmp.b8[1];
			 x[8] = _tmp.b8[0];
		}
	}

protected:
	Endian();
private:
	bool mLittleEndian;
};

} /* namespace ul */

#endif /* UTILITY_ENDIAN_H_ */
