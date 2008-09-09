#ifndef OBJECTMEMSINGLETONPOOL_H
#define OBJECTMEMSINGLETONPOOL_H

namespace ma
{
namespace core
{
	template<template<typename T,typename Mutex> class SmallObjMemPool,
		template<typename T, typename Mutex> class BigObjMemPool,
		typename Mutex>
	struct ObjectMemSingletonPool{};
}
}


#endif