#pragma once

template <class T> class CircleBuffer
{
public:
	CircleBuffer()
	{
		_data       = nullptr;
		_size       = 0;
		_tailIndex  = 0;
	}

	~CircleBuffer()
	{
		delete[] _data;
	}

	void resize(int newSize)
	{
		delete[] _data;

		_data = new T[newSize];
		_size = newSize;
		_tailIndex = 0;
	}

	int size() const
	{
		return _size;
	}

	void push_back(const T &item)
	{
		_tailIndex        = (_tailIndex + 1 > _size - 1) ? _tailIndex + 1 - _size : _tailIndex + 1;
		_data[_tailIndex] = item;
	}

	T& operator[](int index) const
	{
		return _data[(_tailIndex + index + 1) % _size];
	}

private:
	T*	_data;
	int	_size;
	int	_tailIndex;
};