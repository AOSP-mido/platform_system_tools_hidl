/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FmqType.h"

#include <hidl-util/Formatter.h>
#include <android-base/logging.h>

namespace android {

FmqType::FmqType(const char *nsp, const char *name)
    : mNamespace(nsp), mName(name) {
}

void FmqType::addNamedTypesToSet(std::set<const FQName> &) const {
    // do nothing
}

std::string FmqType::fullName() const {
    return mNamespace +
            (mNamespace.empty() ? "" : "::") +
            mName + "<" + mElementType->getCppStackType(true) + ">";
}

std::string FmqType::getCppType(
        StorageMode mode,
        bool) const {

    const std::string base = fullName();

    switch (mode) {
        case StorageMode_Stack:
            return base;

        case StorageMode_Argument:
            return "const " + base + "&";

        case StorageMode_Result:
            return "const " + base + "*";
    }
}

void FmqType::emitReaderWriter(
        Formatter &out,
        const std::string &name,
        const std::string &parcelObj,
        bool parcelObjIsPointer,
        bool isReader,
        ErrorMode mode) const {
    const std::string parentName = "_hidl_" + name + "_parent";

    out << "size_t " << parentName << ";\n\n";

    const std::string parcelObjDeref =
        parcelObj + (parcelObjIsPointer ? "->" : ".");

    if (isReader) {
        out << name
            << " = (const "
            << fullName()
            << " *)"
            << parcelObjDeref
            << "readBuffer("
            << "&"
            << parentName
            << ");\n";

        out << "if ("
            << name
            << " == nullptr) {\n";

        out.indent();

        out << "_hidl_err = ::android::UNKNOWN_ERROR;\n";
        handleError2(out, mode);

        out.unindent();
        out << "}\n\n";
    } else {
        out << "_hidl_err = "
            << parcelObjDeref
            << "writeBuffer(&"
            << name
            << ", sizeof("
            << name
            << "), &"
            << parentName
            << ");\n";

        handleError(out, mode);
    }

    emitReaderWriterEmbedded(
            out,
            0 /* depth */,
            name,
            name /* sanitizedName */,
            isReader /* nameIsPointer */,
            parcelObj,
            parcelObjIsPointer,
            isReader,
            mode,
            parentName,
            "0 /* parentOffset */");
}

void FmqType::emitReaderWriterEmbedded(
        Formatter &out,
        size_t /* depth */,
        const std::string &name,
        const std::string & /* sanitizedName */,
        bool nameIsPointer,
        const std::string &parcelObj,
        bool parcelObjIsPointer,
        bool isReader,
        ErrorMode mode,
        const std::string &parentName,
        const std::string &offsetText) const {
    emitReaderWriterEmbeddedForTypeName(
            out,
            name,
            nameIsPointer,
            parcelObj,
            parcelObjIsPointer,
            isReader,
            mode,
            parentName,
            offsetText,
            fullName(),
            "" /* childName */,
            mNamespace);
}

bool FmqType::isJavaCompatible() const {
    return false;
}

bool FmqType::needsEmbeddedReadWrite() const {
    return true;
}

bool FmqType::resultNeedsDeref() const {
    return true;
}

bool FmqType::isCompatibleElementType(Type *elementType) const {
    return (!elementType->isInterface() && !elementType->needsEmbeddedReadWrite());
}

}  // namespace android
