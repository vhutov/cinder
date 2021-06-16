// Copyright (c) Facebook, Inc. and its affiliates. (http://www.facebook.com)
#include "StrictModules/Objects/type_type.h"

#include "StrictModules/Objects/callable_wrapper.h"
#include "StrictModules/Objects/object_interface.h"
#include "StrictModules/Objects/objects.h"
namespace strictmod::objects {
std::shared_ptr<BaseStrictObject> StrictTypeType::loadAttr(
    std::shared_ptr<BaseStrictObject> obj,
    const std::string& key,
    std::shared_ptr<BaseStrictObject> defaultValue,
    const CallerContext& caller) {
  auto objType = obj->getType();
  auto descr = objType->typeLookup(key, caller);
  if (descr && descr->getTypeRef().isDataDescr()) {
    // data descr found on metatype
    return iGetDescr(
        std::move(descr), std::move(obj), std::move(objType), caller);
  }
  // lookup in dict of obj and subtypes
  std::shared_ptr<StrictType> typ = std::dynamic_pointer_cast<StrictType>(obj);
  assert(typ != nullptr);
  auto dictDescr = typ->typeLookup(key, caller);
  if (dictDescr) {
    return iGetDescr(std::move(dictDescr), nullptr, std::move(typ), caller);
  }
  // lastly, invoke non-data descriptor, if any
  if (descr) {
    return iGetDescr(
        std::move(descr), std::move(obj), std::move(objType), caller);
  }
  return defaultValue;
}

void StrictTypeType::storeAttr(
    std::shared_ptr<BaseStrictObject> obj,
    const std::string& key,
    std::shared_ptr<BaseStrictObject> value,
    const CallerContext& caller) {
  auto objType = obj->getType();
  auto descr = objType->typeLookup(key, caller);
  if (descr && descr->getTypeRef().isDataDescr()) {
    // data descr found on metatype
    iSetDescr(std::move(descr), std::move(obj), std::move(value), caller);
  }

  std::shared_ptr<StrictType> typ = std::dynamic_pointer_cast<StrictType>(obj);
  assert(typ != nullptr);
  if (typ->isImmutable()) {
    caller.error<ImmutableException>(key, "type", typ->getName());
    return;
  }
  checkExternalModification(obj, caller);
  typ->setAttr(key, std::move(value));
}

std::shared_ptr<StrictType> StrictTypeType::recreate(
    std::string name,
    std::weak_ptr<StrictModuleObject> caller,
    std::vector<std::shared_ptr<BaseStrictObject>> bases,
    std::shared_ptr<DictType> members,
    std::shared_ptr<StrictType> metatype,
    bool isImmutable) {
  return createType<StrictTypeType>(
      std::move(name),
      std::move(caller),
      std::move(bases),
      std::move(members),
      std::move(metatype),
      isImmutable);
}

void StrictTypeType::addMethods() {
  addMethodDescr("__call__", StrictType::type__call__);
  addBuiltinFunctionOrMethod("__new__", StrictType::type__new__);
  addMethod("mro", StrictType::typeMro);
}

std::vector<std::type_index> StrictTypeType::getBaseTypeinfos() const {
  std::vector<std::type_index> baseVec = StrictObjectType::getBaseTypeinfos();
  baseVec.emplace_back(typeid(StrictTypeType));
  return baseVec;
}
} // namespace strictmod::objects
