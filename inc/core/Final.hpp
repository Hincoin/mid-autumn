#ifndef MA_FINAL_HPP
#define MA_FINAL_HPP



namespace ma{
	namespace details{
		template<typename T>
		struct final{
		protected:
			final(){}
		};
	}
}


//mixin to produce non-derivable class like java keyword final but 4 byte overhead
#define MA_FINAL(T) virtual private ma::details::final<T>

#endif