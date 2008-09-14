#ifndef MASINGLETON_H
#define MASINGLETON_H

//it's commonly used pattern so make a simple reusable one

namespace ma
{
	namespace core
	{
		//a very simple singleton
		template<typename T>
		class MASimpleSingleton{
		private:
			struct object_creator
			{
				// This constructor does nothing more than ensure that instance()
				//  is called before main() begins, thus creating the static
				//  T object before multithreading race issues can come up.
				object_creator() { MASimpleSingleton<T>::instance(); }
				inline void do_nothing() const { }
			};
			static object_creator create_object;

			MASimpleSingleton();
			MASimpleSingleton(const MASimpleSingleton&);
			MASimpleSingleton& operator=(const MASimpleSingleton&);

		public:
			typedef T object_type;

			// If, at any point (in user code), singleton_default<T>::instance()
			//  is called, then the following function is instantiated.
			static object_type & instance()
			{
				// This is the object that we return a reference to.
				// It is guaranteed to be created before main() begins because of
				//  the next line.
				static object_type obj;

				// The following line does nothing else than force the instantiation
				//  of singleton_default<T>::create_object, whose constructor is
				//  called before main() begins.
				create_object.do_nothing();
				return obj;
			}
		};
		template <typename T>
		typename MASimpleSingleton<T>::object_creator
			MASimpleSingleton<T>::create_object;

	}
}



#endif