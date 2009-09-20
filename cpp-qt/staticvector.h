/***************************************************************************
 * Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ***************************************************************************/

#ifndef UTILS_STATICVECTOR_H
#define UTILS_STATICVECTOR_H

#include <cstdlib>
#include <QtCore/QtGlobal>

namespace Utils
{
	/**
	 * \class StaticVector
	 * \since 2.0
	 *
	 * This class provides a vector data structure (i.e. a list in which the elements occupy adjacent memory positions) that is guaranteed not to change its position in memory until it is explicitly reallocated.
	 *
	 * Its implementation is similar to that of QVector, but with one important difference:
\code
void passMyDataToSomeOtherSubsystem(uint* data, int size);

class MyContainer
{
	QVector<uint*> m_payload;
	public:
		QVector<uint*> payload() const { return m_data; }
		//...
};

MyContainer container;
//...
passMyDataToSomeOtherSubsystem(container.payload().data(), container.payload().size());
\endcode
	 * This code will not work because data() is a non-const method. In the method call, two temporary QVector instances are created (which is normally a cheap operation because of implicit sharing), but the call of QVector::data for the first of these two instances causes this instance to detach from the original instance, thereby copying the data to some other position. The returned data pointer points to this new memory. After the method call, the temporary QVector instances are disposed, which deletes the copied data and invalidates the pointer passed to the other subsystem.
	 *
	 * To see this error in action: http://websvn.kde.org/?view=rev&revision=1007469
	 *
	 * StaticVectors are implicitly shared for performance reasons, but do not detach from the shared data on changes (for the reason described above). To keep copied instances from modifying the original data, only the one instance that allocated the data object is allowed to change the data (any write attempt by other instances will trigger an assertion and crash the program, at least to the extent possible in C++).
	 *
	 * \warning Unlike QVector, StaticVector does not resize automatically.
	 */
	template<typename T> class StaticVector
	{
		private:
			struct Data //the internal data class for implicit sharing
			{
				T* m_base;
				int m_size;
				StaticVector* m_writeAccessVector; //a pointer to the only vector that is allowed to write on this data
				int m_refCounter; //to determine when to delete this object
			};
			Data* m_data;
		public:
			///Initializes a new shared vector that initially is not able to hold any data. (You have to call resize() first.)
			inline StaticVector();
			///Constructs a read-only copy of \a other.
			inline StaticVector(const Kolf::StaticVector<T>& other);
			///Destroys the vector.
			inline ~StaticVector();
			///Resizes the vector.
			///\warning This discards all values saved in the vector. The elements are not initialized for performance reasons.
			inline void resize(int size);

			///Returns the number of items in this vector.
			inline int size() const;
			///Same as size().
			inline int count() const;
			///Returns a pointer to the data stored in the vector. The pointer is explicitly non-const because some libraries require non-const pointers to be passed to them, even if they do not modify the data (most notably ODE and OpenGL).
			inline T* data() const;

			///Returns the value at index position \a i in the vector. If the index \a i is out of bounds, returns \a defaultValue instead.
			inline T value(int i, const T& defaultValue) const;
			///Returns the item at index position \a i in the vector. \a i must be a valid index position in the vector (i.e., 0 <= \a i < size()).
			inline const T& at(int i) const;
			///Same as at(), but this operation is only allowed if the data in this vector is writable (i.e., the data was created by a resize() call in this very instance).
			inline T& operator[](int i);
	};
}

//Because it's a template, everything has to be implemented in this header.

template<typename T> Kolf::StaticVector<T>::StaticVector()
{
	m_data = 0;
}

template<typename T> Kolf::StaticVector<T>::StaticVector(const Kolf::StaticVector<T>& other)
{
	m_data = other.m_data;
	if (m_data)
		++m_data->m_refCounter;
}

template<typename T> Kolf::StaticVector<T>::~StaticVector()
{
	if (m_data && --m_data->m_refCounter == 0)
	{
		free(m_data->m_base);
		delete m_data;
	}
}

template<typename T> void Kolf::StaticVector<T>::resize(int size)
{
	//detach from old data
	if (m_data && --m_data->m_refCounter == 0)
	{
		free(m_data->m_base);
		delete m_data;
	}
	//create new data if necessary
	if (size == 0)
		m_data = 0;
	else
	{
		m_data = new Data;
		m_data->m_base = reinterpret_cast<T*>(malloc(size * sizeof(T)));
		m_data->m_size = size;
		m_data->m_writeAccessVector = this;
		m_data->m_refCounter = 1; //this instance holds the only reference currently
	}
}

template<typename T> int Kolf::StaticVector<T>::size() const
{
	return m_data ? m_data->m_size : 0;
}

template<typename T> int Kolf::StaticVector<T>::count() const
{
	return size();
}

template<typename T> T* Kolf::StaticVector<T>::data() const
{
	return m_data ? m_data->m_base : 0;
}

template<typename T> T Kolf::StaticVector<T>::value(int i, const T& defaultValue) const
{
	if (!m_data)
		return defaultValue;
	if (i < 0 || i >= m_data->m_size)
		return defaultValue;
	return m_data->m_base[i];
}

template<typename T> const T& Kolf::StaticVector<T>::at(int i) const
{
	Q_ASSERT(m_data);
	Q_ASSERT(i >= 0 && i < m_data->m_size);
	return m_data->m_base[i];
}

template<typename T> T& Kolf::StaticVector<T>::operator[](int i)
{
	Q_ASSERT(m_data);
	Q_ASSERT(i >= 0 && i < m_data->m_size);
	if (m_data->m_writeAccessVector == this)
		return m_data->m_base[i];
	else
		return *((T*)0); //If no write access is allowed, fail loudly and early.
}

#endif //UTILS_STATICVECTOR_H
