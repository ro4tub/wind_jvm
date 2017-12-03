/*
 * jni.hpp
 *
 *  Created on: 2017年11月19日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_NATIVE_NATIVE_HPP_
#define INCLUDE_NATIVE_NATIVE_HPP_

#include <string>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <iostream>

using std::wstring;
using std::unordered_map;
using std::function;

// Java Native Interface Spec. [$ JNI types]
// ps: this jni is not real jni. It's target is only to support Native Method.

#define THD L"Ljava/lang/Thread;"
#define OBJ L"Ljava/lang/Object;"
#define STE L"Ljava/lang/StackTraceElement;"
#define STR L"Ljava/lang/String;"
#define FLE L"Ljava/io/File;"
#define IPS L"Ljava/io/InputStream;"
#define PRS L"Ljava/io/PrintStream;"
#define PRO L"Ljava/util/Properties;"
#define CLS L"Ljava/lang/Class;"
#define CPL L"Lsun/reflect/ConstantPool;"
#define JCL L"Ljava/lang/ClassLoader;"
#define FLD L"Ljava/lang/reflect/Field;"
#define MHD L"Ljava/lang/reflect/Method;"
#define CTR L"Ljava/lang/reflect/Constructor;"
#define PD  L"Ljava/security/ProtectionDomain;"
#define PA  L"Ljava/security/PrivilegedAction;"
#define PEA L"Ljava/security/PrivilegedExceptionAction;"
#define ACC L"Ljava/security/AccessControlContext;"
#define VOD L"Ljava/lang/Void;"
#define BA  "[B"

#define STRING				L"java/lang/String"
#define FIELD				L"java/lang/reflect/Field"
#define CONSTRUCTOR			L"java/lang/reflect/Constructor"
#define CLASS				L"java/lang/Class"
#define THREAD				L"java/lang/Thread"
#define FILEOUTPUTSTREAM		L"java/io/FileOutputStream"
#define FILEDESCRIPTOR		L"java/io/FileDescriptor"
#define JFILE				L"java/io/File"

void init_native();

// 说明：所有的 native 方法我全部设置成为签名是 void (*)(list<Oop *> & _stack) 的形式了。避免了签名不同调用困难的问题。这样就很好啦～参数和返回值全都会被压入 _stack 中～～
void *find_native(const wstring & klass_name, const wstring & signature);	// get a native method <$signature> in klass <$klass_name>. maybe return nullptr...

#endif /* INCLUDE_NATIVE_NATIVE_HPP_ */
