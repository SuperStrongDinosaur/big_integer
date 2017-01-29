//
//  any.h
//  cpp
//
//  Created by Alex Shirvinsky on 29.01.17.
//  Copyright © 2017 Alex Shirvinsky. All rights reserved.
//

#ifndef any_h
#define any_h

#include <typeinfo>
#include <typeindex>

class any {
public:
    any(): data(0) {}
    
    template<typename T>
    any(const T& val) :
    data(new holder<typename std::remove_cv<typename std::decay<const T>::type>::type>(val)){}
    
    any(const any& other) : data(other.data ? other.data->clone() : 0) {}
    
    any(any&& other) : data(other.data) {}
    
    ~any() {}
    
    any& swap(any& other) {
        std::swap(data, other.data);
        return *this;
    }
    
    any& operator=(any&& other) {
        other.swap(*this);
        return *this;
    }
   
    template <class T>
    any& operator=(T&& other) {
        any(static_cast<T&&>(other)).swap(*this);
        return *this;
    }
    
    bool empty() const {
        return !data;
    }
    
    void clear() {
        any().swap(*this);
    }
    
    const std::type_index& type() const {
        return std::move(data ? data->type() : typeid(void));
    }
    
    template<typename T>
    friend T* any_cast(any*);
    template<typename T>
    friend inline const T* any_cast(const any*);
    template<typename T>
    friend T any_cast(any& oper);
    template<typename T>
    friend inline T any_cast(const any& oper);
    template<typename T>
    friend inline T any_cast(any&& oper);
    
private:
    struct placeholder {
        virtual ~placeholder() {}
        virtual const std::type_index& type() const = 0;
        virtual placeholder* clone() const = 0;
    };
    
    template<typename T>
    class holder : public placeholder {
    public:
        holder(const T & val) : held(val) {}
        
        holder(T&& val) : held(static_cast<T&&>(val)) {}
        
        virtual const std::type_index& type() const {
            return std::move(typeid(T));
        }
        
        virtual placeholder* clone() const {
            return std::move(new holder(held));
        }

        T held;
    };
    
    std::shared_ptr<placeholder> data;
};

inline void swap(any& a, any& b) {
    a.swap(b);
}

template<typename T>
T* any_cast(any* oper) {
    return oper && oper->type() == typeid(T) ?
    &static_cast<any::holder<typename std::remove_cv<T>::type>*>(oper->data.get())->held : 0;
}

template<typename T>
inline const T* any_cast(const any* oper) {
    return any_cast<T>(const_cast<any*>(oper));
}

template<typename T>
T any_cast(any& oper) {
    typedef typename std::remove_reference<T>::type non;
    non* res = any_cast<non>(&oper);
    return *res;
}

template<typename T>
inline T any_cast(const any& oper) {
    typedef typename std::remove_reference<T>::type non;
    return any_cast<const non&>(const_cast<any&>(oper));
}

template<typename T>
inline T any_cast(any&& oper) {
    return any_cast<T>(oper);
}

#endif /* any_h */