#include <impatiens/Runtime.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace impatiens;

// OpaqueFunctionHandle

OpaqueFunctionHandle::OpaqueFunctionHandle() :
    function(nullptr), uuid(traits::nullUUID), isValid(false) {}

OpaqueFunctionHandle::OpaqueFunctionHandle(OpaqueFunction const* func, traits::UUIDType uuid) :
    function(func), uuid(uuid), isValid(true) {}

OpaqueFunctionHandle::~OpaqueFunctionHandle() {
    Runtime::get().returnFunctionHandle(this);
}

OpaqueFunctionHandle::OpaqueFunctionHandle(OpaqueFunctionHandle&& other) noexcept :
    function(other.function), isValid(other.isValid), uuid(other.uuid) {
    other.isValid = false;
    other.function = nullptr;
    other.uuid = traits::nullUUID;
}

OpaqueFunctionHandle& OpaqueFunctionHandle::operator=(OpaqueFunctionHandle&& other) noexcept {
    if (this != &other) {
        function = other.function;
        isValid = other.isValid;
        uuid = other.uuid;
        other.isValid = false;
        other.function = nullptr;
        other.uuid = traits::nullUUID;
    }
    return *this;
}

// string map stuff

namespace {

    inline uint64_t fnv1aHash(char const* str) {
        uint64_t hash = 0xcbf29ce484222325;
        while (*str) {
            hash ^= *str++;
            hash *= 0x100000001b3;
        }
        return hash;
    }

    inline uint64_t fnv1aHash(std::string_view str) {
        uint64_t hash = 0xcbf29ce484222325;
        for (char c : str) {
            hash ^= c;
            hash *= 0x100000001b3;
        }
        return hash;
    }

    struct StringHash {
        using is_transparent = void;

        size_t operator()(char const* str) const {
            return fnv1aHash(str);
        }

        size_t operator()(std::string_view str) const {
            return fnv1aHash(str);
        }

        size_t operator()(std::string const& str) const {
            return fnv1aHash(str);
        }
    };
}

// Runtime

class Runtime::Impl {
public:
    struct FunctionContent {
        traits::UUIDType uuid = traits::nullUUID;
        std::vector<OpaqueFunction const*> functions = {nullptr};

        FunctionContent(traits::UUIDType uuid) : uuid(uuid) {}
    };

    struct StackContent {
        traits::UUIDType uuid = traits::nullUUID;
        FunctionContent* functionContent = nullptr;
        size_t currentIndex = 0;
    };

    static thread_local inline std::vector<StackContent> callStack;

    std::unordered_map<std::string, traits::UUIDType, StringHash, std::equal_to<>> nameToUUIDMap;

    std::unordered_map<traits::UUIDType, std::unique_ptr<FunctionContent>> functionMap;
};

Runtime::Runtime() : m_impl(std::make_unique<Impl>()) {}

Runtime::~Runtime() = default;

Runtime& Runtime::get() {
    static Runtime instance;
    return instance;
}

void Runtime::registerMapping(traits::UUIDType uuid, std::string name) {
    std::cout << "Registering mapping: " << name << " -> " << (void*)uuid << std::endl;
    m_impl->nameToUUIDMap.insert({std::move(name), uuid});
    m_impl->functionMap.insert({uuid, std::make_unique<Impl::FunctionContent>(uuid)});
}

OpaqueFunctionHandle Runtime::getNextFunction(traits::UUIDType uuid) {
    // check top of the stack, if the uuid does not match, put a new stack content
    // otherwise, increment the index in current index
    // std::cout << "Getting next function for UUID: " << (void*)uuid << std::endl;
    if (m_impl->callStack.empty() || m_impl->callStack.back().uuid != uuid) {
        auto iterator = m_impl->functionMap.find(uuid);
        if (iterator == m_impl->functionMap.end()) {
            // function not found, how did that happen?
            return OpaqueFunctionHandle{};
        }
        auto functionContent = iterator->second.get();

        auto stackContent = Impl::StackContent{uuid, functionContent, 0};
        m_impl->callStack.push_back(stackContent);

        // std::cout << "Found next function (new): " << (void*)functionContent->functions[0] << " -> "
        //   << (void*)uuid << std::endl;

        return OpaqueFunctionHandle{functionContent->functions[0], uuid};
    }
    else {
        auto& stackContent = m_impl->callStack.back();
        stackContent.currentIndex++;

        auto functionContent = stackContent.functionContent;
        auto functionIndex = stackContent.currentIndex % functionContent->functions.size();

        // std::cout << "Found next function (old): " << (void*)functionContent->functions[functionIndex]
        //           << " -> " << (void*)uuid << std::endl;

        return OpaqueFunctionHandle{functionContent->functions[functionIndex], uuid};
    }
}

void Runtime::returnFunctionHandle(OpaqueFunctionHandle const* handle) {
    if (!handle->isValid) return;

    if (m_impl->callStack.empty()) {
        // stack underflow, how did that happen?
        return;
    }

    // std::cout << "Popping function: " << (void*)handle->function << " -> " << (void*)handle->uuid
    //           << std::endl;
    // std::cout << "null uuid: " << (void*)traits::nullUUID << std::endl;

    // check top of the stack, if the uuid matches, decrement the index, otherwise
    // just pop the stack content
    if (m_impl->callStack.back().uuid == handle->uuid) {
        auto& stackContent = m_impl->callStack.back();
        if (stackContent.currentIndex == 0) {
            m_impl->callStack.pop_back();
        }
        else {
            stackContent.currentIndex--;
        }
    }
}

bool Runtime::insertHook(std::string_view name, OpaqueFunction const* function) {
    std::cout << "Inserting hook: " << (void*)function << " -> " << name << std::endl;

    auto iterator = m_impl->nameToUUIDMap.find(name);
    if (iterator == m_impl->nameToUUIDMap.end()) {
        // function not found, how did that happen?
        return false;
    }
    auto uuid = iterator->second;
    std::cout << "Found UUID: " << (void*)uuid << std::endl;

    auto functionContent = m_impl->functionMap.find(uuid);
    if (functionContent == m_impl->functionMap.end()) {
        // function not found, how did that happen?
        return false;
    }

    functionContent->second->functions.insert(functionContent->second->functions.begin(), function);
    return true;
}