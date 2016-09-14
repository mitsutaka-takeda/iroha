#include "javaVM.hpp"

void Java_SmartContract_save(JNIEnv *env,jobject thiz,jstring key,jstring value){
    const char *keyChar   = env->GetStringUTFChars(   key,0);
    const char *valueChar = env->GetStringUTFChars( value,0);

    std::cout << "[" << keyChar <<", "<< valueChar << std::endl;

    env->ReleaseStringUTFChars(  key,   keyChar);
    env->ReleaseStringUTFChars(value, valueChar);
}

std::unique_ptr<JavaContext> createVM(std::string contractName){
  JNIEnv* env;
  JavaVM* jvm;

  if(getenv("IROHA_HOME") == nullptr){
    std::cout << "You must set IROHA_HOME!" << std::endl;
    return nullptr;
  }
  std::cout << "java -Djava.class.path="+(std::string)getenv("IROHA_HOME")+"/smartContract/"+contractName+"/  " << contractName << std::endl;

  JavaVMOption options[3];
  //options[0].optionString = (char*)"-Djava.security.manager -Djava.security.policy=policy.txt -Djava.class.path=./contract/";
  options[1].optionString =  (char*)"-Djava.security.manager";
  options[2].optionString =  (char*)"-Djava.security.policy=policy.txt";
  options[0].optionString = (char*)("-Djava.class.path="+(std::string)getenv("IROHA_HOME")+"/smartContract/"+contractName+"/").c_str();

  JavaVMInitArgs vm_args;
  vm_args.version = JNI_VERSION_1_6;
  vm_args.options = options;
  vm_args.nOptions = 3;
  //vm_args.ignoreUnrecognized = true;

  int res = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
  if(res) {
    std::cout << "cannot run JavaVM : "<< res << std::endl;
    return nullptr;
  }
  std::unique_ptr<JavaContext> ptr(new JavaContext(
    env,
    jvm,
    vm_args,
    std::move(contractName)
  ));
  return ptr;
}

void execVM(const std::unique_ptr<JavaContext>& context){
  int res;

  jclass cls = context->env->FindClass(context->name.c_str());
  if(cls == 0){
    std::cout << "could not found class : "<< context->name.c_str() << std::endl;
    return;
  }

  jmethodID cns = context->env->GetMethodID(cls, "<init>", "()V");
  if(cns == NULL){
    std::cout << "could not get <init> method." << std::endl;
    return;
  }
  jobject obj = context->env->NewObject(cls, cns);

  jmethodID mid = context->env->GetStaticMethodID(cls, "remit", "(Ljava/util/Map;)V");
  if(mid == NULL){
    std::cout << "could not get method : " << "remit" << std::endl;
    return;
  }

  context->env->CallVoidMethod(obj, mid);

  res = context->jvm->DestroyJavaVM();
  if(res){
    std::cout << "could not destroy JavaVM : " << res << std::endl;
  }
}

