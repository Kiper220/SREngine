//
// Created by Monika on 21.07.2022.
//

#ifndef SRENGINE_FUNCTION_H
#define SRENGINE_FUNCTION_H

#include <Utils/stdInclude.h>

namespace SR_HTYPES_NS {
    template <typename UnusedType>
    class Function;

    template <typename ReturnType, typename... ArgumentTypes>
    class Function <ReturnType (ArgumentTypes...)>
    {
        class function_holder_base;
    #ifdef SR_ANDROID
        using invoker_t = std::unique_ptr<function_holder_base>;
    #else
        using invoker_t = std::auto_ptr<function_holder_base>;
    #endif
    public:
        typedef ReturnType signature_type(ArgumentTypes...);

        Function()
            : mInvoker()
        { }

        template <typename FunctionT> Function(FunctionT f)
            : mInvoker(new free_function_holder<FunctionT>(f))
        { }

        Function(Function&& function) noexcept {
            mInvoker = function.mInvoker->clone();
        }

        Function& operator=(Function&& function) noexcept {
            mInvoker = function.mInvoker->clone();
            return *this;
        }

        template <typename FunctionType, typename ClassType>
        Function(FunctionType ClassType::* f)
            : mInvoker(new member_function_holder<FunctionType, ArgumentTypes ...>(f))
        { }

        Function(const Function & other)
            : mInvoker(other.mInvoker->clone())
        { }

        Function& operator=(const Function& other) {
            mInvoker = other.mInvoker->clone();
            return *this;
        }

        ReturnType operator()(ArgumentTypes... args) const noexcept {
            return mInvoker->invoke(args...);
        }

        operator bool() const {
            return mInvoker.get();
        }

    private:
        class function_holder_base
        {
        public:
            function_holder_base() = default;
            virtual ~function_holder_base() = default;

            virtual ReturnType invoke(ArgumentTypes... args) = 0;
            virtual invoker_t clone() = 0;

        private:
            function_holder_base(const function_holder_base &);
            void operator=(const function_holder_base &);

        };

        template <typename FunctionT>
        class free_function_holder : public function_holder_base
        {
        public:
            free_function_holder(FunctionT func)
                : function_holder_base()
                , mFunction(func)
            { }

            virtual ReturnType invoke(ArgumentTypes ... args) {
                return mFunction(args ...);
            }

            virtual invoker_t clone() {
                return invoker_t(new free_function_holder(mFunction));
            }

        private:
            FunctionT mFunction;

        };

        template <typename FunctionType, typename ClassType, typename ... RestArgumentTypes>
        class member_function_holder : public function_holder_base
        {
        public:
            typedef FunctionType ClassType::* member_function_signature_t;

            member_function_holder(member_function_signature_t f)
                : mFunction(f)
            { }

            virtual ReturnType invoke(ClassType obj, RestArgumentTypes... restArgs) {
                return (obj.*mFunction)(restArgs...);
            }

            virtual invoker_t clone() {
                return invoker_t(new member_function_holder(mFunction));
            }

        private:
            member_function_signature_t mFunction;

        };

        invoker_t mInvoker;
    };
}

#endif //SRENGINE_FUNCTION_H
