#include "runtime/method.hpp"
#include "runtime/klass.hpp"
#include "utils/utils.hpp"
#include "runtime/oop.hpp"
#include "runtime/bytecodeEngine.hpp"
#include "native/java_lang_Class.hpp"
#include "classloader.hpp"

Method::Method(shared_ptr<InstanceKlass> klass, method_info & mi, cp_info **constant_pool) : klass(klass) {
	assert(constant_pool[mi.name_index-1]->tag == CONSTANT_Utf8);
	name = ((CONSTANT_Utf8_info *)constant_pool[mi.name_index-1])->convert_to_Unicode();
	assert(constant_pool[mi.descriptor_index-1]->tag == CONSTANT_Utf8);
	descriptor = ((CONSTANT_Utf8_info *)constant_pool[mi.descriptor_index-1])->convert_to_Unicode();
	access_flags = mi.access_flags;

	// move!!! important!!!
	this->attributes = mi.attributes;
	mi.attributes = nullptr;

	// set to 0!!! important!!!
	this->attributes_count = mi.attributes_count;
	mi.attributes_count = 0;

	for(int i = 0; i < this->attributes_count; i ++) {
		int attribute_tag = peek_attribute(this->attributes[i]->attribute_name_index, constant_pool);
		switch (attribute_tag) {	// must be 1, 3, 6, 7, 13, 14, 15, 16, 17, 18, 19, 20, 22 for Method.
								// must be 2, 10, 11, 12, 18, 19 for Code attribute.
			case 1:{	// Code
				code = (Code_attribute *)this->attributes[i];
				for (int pos = 0; pos < code->attributes_count; pos ++) {
					int code_attribute_tag = peek_attribute(code->attributes[pos]->attribute_name_index, constant_pool);
					switch (code_attribute_tag) {
						case 18:{		// RuntimeVisibleTypeAnnotation
							auto enter_ptr = ((RuntimeVisibleTypeAnnotations_attribute *)code->attributes[pos]);
							this->Code_num_RuntimeVisibleTypeAnnotations = enter_ptr->num_annotations;
							this->Code_rvta = (TypeAnnotation *)malloc(sizeof(TypeAnnotation) * this->Code_num_RuntimeVisibleTypeAnnotations);
							for (int pos = 0; pos < this->Code_num_RuntimeVisibleTypeAnnotations; pos ++) {
								constructor(&this->Code_rvta[pos], constant_pool, enter_ptr->annotations[pos]);
							}
							break;
						}
						case 2:
						case 10:
						case 11:
						case 12:
						case 19:{
							break;
						}
						default:{
							std::wcerr << "Annotations are TODO! attribute_tag == " << code_attribute_tag << " in Method: [" << name << "]" << std::endl;
							assert(false);
						}
					}
				}
				break;
			}
			case 3:{	// Exception
				exceptions = (Exceptions_attribute *)this->attributes[i];
				break;
			}
			case 7:{	// Signature
				signature_index = ((Signature_attribute *)this->attributes[i])->signature_index;
				break;
			}
			case 14:{	// RuntimeVisibleAnnotation
				auto enter = ((RuntimeVisibleAnnotations_attribute *)this->attributes[i])->parameter_annotations;
				this->rva = (Parameter_annotations_t *)malloc(sizeof(Parameter_annotations_t));
				constructor(this->rva, constant_pool, enter);
				break;
			}
			case 16:{	// RuntimeVisibleTypeAnnotation
				auto enter_ptr = ((RuntimeVisibleParameterAnnotations_attribute *)this->attributes[i]);
				this->num_RuntimeVisibleParameterAnnotation = enter_ptr->num_parameters;
				this->rvpa = (Parameter_annotations_t *)malloc(sizeof(Parameter_annotations_t) * this->num_RuntimeVisibleParameterAnnotation);
				for (int pos = 0; pos < this->num_RuntimeVisibleParameterAnnotation; pos ++) {
					constructor(&this->rvpa[pos], constant_pool, enter_ptr->parameter_annotations[pos]);
				}
				break;
			}
			case 18:{		// RuntimeVisibleTypeAnnotation
				auto enter_ptr = ((RuntimeVisibleTypeAnnotations_attribute *)this->attributes[i]);
				this->num_RuntimeVisibleTypeAnnotations = enter_ptr->num_annotations;
				this->rvta = (TypeAnnotation *)malloc(sizeof(TypeAnnotation) * this->num_RuntimeVisibleTypeAnnotations);
				for (int pos = 0; pos < this->num_RuntimeVisibleTypeAnnotations; pos ++) {
					constructor(&this->rvta[pos], constant_pool, enter_ptr->annotations[pos]);
				}
				break;
			}
			case 20:{		// Annotation Default
				auto element_value = ((AnnotationDefault_attribute *)this->attributes[i])->default_value;
				this->ad = (Element_value *)malloc(sizeof(Element_value));
				constructor(this->ad, constant_pool, element_value);
				break;
			}
			case 6:
			case 13:
			case 15:
			case 17:
			case 19:
			case 22:
			{	// do nothing
				break;
			}
			default:{
				std::cerr << "attribute_tag == " << attribute_tag << std::endl;
				assert(false);
			}
		}

	}
}


wstring Method::parse_signature()
{
	assert(signature_index != 0);
	auto _pair = (*klass->get_rtpool())[signature_index];
	assert(_pair.first == CONSTANT_Utf8);
	return boost::any_cast<wstring>(_pair.second);
}

vector<MirrorOop *> Method::if_didnt_parse_exceptions_then_parse()
{
	if (!parsed) {
		parsed = true;
		for (int i = 0; i < exceptions->number_of_exceptions; i ++) {
			auto rt_pool = this->klass->get_rtpool();
			assert((*rt_pool)[exceptions->exception_index_table[i]-1].first == CONSTANT_Class);
			auto excp_klass = boost::any_cast<shared_ptr<Klass>>((*rt_pool)[exceptions->exception_index_table[i]-1].second);
			exceptions_tb[excp_klass->get_name()] = excp_klass;
		}
	}

	vector<MirrorOop *> v;
	for (auto iter : exceptions_tb) {
		v.push_back(iter.second->get_mirror());
	}
	return v;
}

vector<MirrorOop *> Method::parse_argument_list()
{
	vector<MirrorOop *> v;
	vector<wstring> args = BytecodeEngine::parse_arg_list(this->descriptor);
	for (int i = 0; i < args.size(); i ++) {
		if (args[i].size() == 1) {		// primitive type
			switch (args[i][0]) {
				case L'Z':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"Z"));
					break;
				}
				case L'B':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"B"));
					break;
				}
				case L'S':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"S"));
					break;
				}
				case L'C':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"C"));
					break;
				}
				case L'I':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"I"));
					break;
				}
				case L'F':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"F"));
					break;
				}
				case L'J':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"J"));
					break;
				}
				case L'D':{
					v.push_back(java_lang_class::get_basic_type_mirror(L"D"));
					break;
				}
				default:{
					assert(false);
				}
			}
		} else if (args[i][0] == L'L') {	// InstanceOop type
			auto klass = BootStrapClassLoader::get_bootstrap().loadClass(args[i].substr(1, args[i].size() - 2));
			assert(klass != nullptr);
			v.push_back(klass->get_mirror());
		} else {		// ArrayType
			assert(args[i][0] == L'[');
//			auto klass = BootStrapClassLoader::get_bootstrap().loadClass(args[i]);....
		}
	}
}
