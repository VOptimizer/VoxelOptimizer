/*
 * MIT License
 *
 * Copyright (c) 2020 Christian Tost
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <string.h>
#include <vector>

/**
 * @brief JSON exception type for faster exception processing.
 */
enum class JSONErrorType
{
    NAME_ALREADY_EXITS, //!< Occurred if a key already exists.
    NAME_NOT_FOUND,     //!< Occurred if a key is not found.
    INVALID_JSON_OBJECT,//!< Occurred if a json object is not valid.
    INVALID_ARRAY,      //!< Occurred if a json array is not valid.
    INVALID_TYPE,       //!< Occurred if an unknown type is found. Formally if '"' missing around string.
    INVALID_CAST,       //!< Occurred if a type couldn't cast to the given one.
    EXPECTED_BOOL,      //!< Occurred if a bool is expected but not found.
    EXPECTED_NULL,      //!< Occurred if a null type is expected but not found.
    EXPECTED_NUM,       //!< Occurred if a num is expected but not found.
    WRONG_PLACED_SEPERATOR, //!< Occurred if a seprator is wrong placed.
    MISSING_KEY,            //!< Value found but key expected.
    MISSING_VALUE,          //!< Key found but value missing.
    MISSING_SEPERATOR       //!< Occurred if a ',' is missing before a key or ':' is missing before value.
};

/**
 * @brief JSON value types.
 */
enum class JsonType
{
    UNKNOWN,
    NIL,
    OBJECT,
    ARRAY,
    BOOLEAN,
    INTEGER,
    DOUBLE,
    STRING
};

class CJSONException : public std::exception
{
    public:
        CJSONException() {}
        CJSONException(JSONErrorType Type) : m_ErrType(Type) {}
        CJSONException(const std::string &Msg, JSONErrorType Type) : m_Msg(Msg), m_ErrType(Type) {}

        const char *what() const noexcept override
        {
            return m_Msg.c_str();
        }

        JSONErrorType GetErrType() const noexcept
        {
            return m_ErrType;
        }

    private:
        std::string m_Msg;
        JSONErrorType m_ErrType;
};

class CJSON
{
    /**-----------------------------------------Blackmagic for SFINAE-----------------------------------------**/

    //Concept from https://dev.krzaq.cc/post/checking-whether-a-class-has-a-member-function-with-a-given-signature/
    template<class T>
    struct has_push_back
    {
        private:
            using Type = typename std::remove_pointer<T>::type;

            template<class C> static auto Test(typename C::value_type *p) -> decltype(std::declval<C>().push_back(*p), std::true_type()) { return std::true_type(); }
            template<class> static std::false_type Test(...) { return std::false_type(); }
        public:
            static const bool value = std::is_same<std::true_type, decltype(Test<Type>(nullptr))>::value;
    };

    template<class T>
    struct has_push_front
    {
        private:
            using Type = typename std::remove_pointer<T>::type;

            template<class C> static auto Test(typename C::value_type *p) -> decltype(std::declval<C>().push_front(*p), std::true_type()) { return std::true_type(); }
            template<class> static std::false_type Test(...) { return std::false_type(); }
        public:
            static const bool value = std::is_same<std::true_type, decltype(Test<Type>(nullptr))>::value;
    };

    template<class T>
    struct has_begin_end
    {
        private:
            using Type = typename std::remove_pointer<T>::type;

            template<class C> static auto TestBegin(typename C::const_iterator *) -> decltype(static_cast<typename C::const_iterator>(std::declval<C>().begin()), std::true_type())  { return std::true_type(); }
            template<class> static std::false_type TestBegin(...) { return std::false_type(); }

            template<class C> static auto TestEnd(typename C::const_iterator *) -> decltype(static_cast<typename C::const_iterator>(std::declval<C>().end()), std::true_type())  { return std::true_type(); }
            template<class> static std::false_type TestEnd(...) { return std::false_type(); }
        public:
            static const bool value = std::is_same<std::true_type, decltype(TestBegin<Type>(nullptr))>::value && std::is_same<std::true_type, decltype(TestEnd<Type>(nullptr))>::value;
    }; 

    template<class T>
    struct is_map : std::false_type {};

    template<class v>
    struct is_map<std::map<std::string, v>> : std::true_type {};

    template<class v>
    struct is_map<std::unordered_map<std::string, v>> : std::true_type {};

    template<class T>
    struct is_multimap : std::false_type {};

    template<class v>
    struct is_multimap<std::multimap<std::string, v>> : std::true_type {};

    template<class v>
    struct is_multimap<std::unordered_multimap<std::string, v>> : std::true_type {};

    template<class T>
    struct is_shared_ptr : std::false_type {};

    template<class T>
    struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

    template<class T>
    struct is_pointer_type
    {
        static const bool value = std::is_pointer<T>::value || is_shared_ptr<T>::value;
    };

    template<class T>
    struct pointer_type 
    {
        using type = typename std::remove_pointer<T>::type;
    };

    template<class T>
    struct pointer_type<std::shared_ptr<T>>
    {
        using type = T;
    };

    /**-----------------------------------------Blackmagic for SFINAE-----------------------------------------**/

    public:
        CJSON(/* args */) {}

        /**
         * @brief Serializes an object to a json object.
         * 
         * To serialize an object you need to implement a serialize method.
         * @code
         *      class YourClass
         *      {
         *          public:
         *              ...
         *              void Serialize(CJSON &json) const
         *              {
         *                  ...
         *                  json.AddPair("yourAttr", m_YourAttr);   //Adds an attribute to the json object.
         *                  ...
         *              }
         *              ...
         *      };
         * @endcode
         * 
         * @param obj: Object for serialization.
         * @note This method handles only non-pointer objects, which are no STL containers.
         * 
         * @return Returns a string which contains the json object.
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && !has_begin_end<T>::value>::type* = nullptr>
        inline std::string Serialize(const T &obj)
        {
            static_assert(std::is_class<T>::value, "Please use structs or objects!");
            m_Values.clear();

            obj.Serialize(*this);
            return Serialize();
        }

        /**
         * @brief Serializes STL container types except std::string, sets and queues.
         * 
         * @param obj: Container for serialization.
         * @note Theoretically you can serialize any container, which has an begin() and end() method who returns a const_iterator.
         * 
         * @return Returns a json array.
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && has_begin_end<T>::value && !std::is_same<T, std::string>::value>::type* = nullptr>
        inline std::string Serialize(const T &obj)
        {
            m_Values.clear();
            return ValueToString(obj);
        }

        /**
         * @brief Serializes a pointer object to a json object.
         * 
         * To serialize a pointer object you need to implement a serialize method.
         * @code
         *      class YourClass
         *      {
         *          public:
         *              ...
         *              void Serialize(CJSON &json) const
         *              {
         *                  ...
         *                  json.AddPair("yourAttr", m_YourAttr);   //Adds an attribute to the json object.
         *                  ...
         *              }
         *              ...
         *      };
         * @endcode
         * 
         * @param obj: Object for serialization.
         * @note This method handles only pointer objects or std::shared_ptr objects, which are no STL containers.
         * 
         * @return Returns a string which contains the json object.
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<is_pointer_type<T>::value && !has_begin_end<T>::value>::type* = nullptr>
        inline std::string Serialize(const T obj)
        {
            static_assert(std::is_class<typename std::remove_pointer<T>::type>::value, "Please use structs or objects!");
            m_Values.clear();

            obj->Serialize(*this);
            return Serialize();
        }

        /**
         * @brief Serializes the data which was added without an object.
         * 
         * @return Returns a string which contains the json object.
         */
        inline std::string Serialize()
        {
            std::string Ret;

            for (auto &&e : m_Values)
            {
                if(!Ret.empty())
                    Ret += ',';

                Ret += '"' + e.first + "\":" + e.second;
            }

            m_Values.clear();
            return '{' + Ret + '}';
        }

        /**
         * @brief Deserializes a json object to a already existing object.
         * 
         * To deserialize a json object you need to implement a Deserialize method.
         * @code
         *      class YourClass
         *      {
         *          public:
         *              ...
         *              void Deserialize(CJSON &json)
         *              {
         *                  ...
         *                  m_YourAttr = json.GetValue<std::string>("yourAttr");   //Gets the value of a json key.
         *                  ...
         *              }
         *              ...
         *      };
         * @endcode
         * 
         * @param json: JSON for deserialization.
         * @param Obj: Object wich will receive the data.
         * 
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!is_map<T>::value && !is_multimap<T>::value && !has_begin_end<T>::value>::type* = nullptr>
        inline void Deserialize(const std::string &json, T *Obj)
        {
            static_assert(std::is_class<typename pointer_type<T>::type>::value, "Please use structs or objects!");

            ParseObject(json);

            Obj->Deserialize(*this);
            m_Values.clear();
        }

        /**
         * @brief Deserializes a json object to a pointer object.
         * 
         * To deserialize a json object you need to implement a Deserialize method, also you need a standard constructor.
         * @code
         *      class YourClass
         *      {
         *          public:
         *              YourClass() {} //<- Necessary
         * 
         *              ...
         *              void Deserialize(CJSON &json)
         *              {
         *                  ...
         *                  m_YourAttr = json.GetValue<std::string>("yourAttr");   //Gets the value of a json key.
         *                  ...
         *              }
         *              ...
         *      };
         * @endcode
         * 
         * @param json: JSON for deserialization.
         * @note This method returns only pointer objects or std::shared_ptr objects.
         * 
         * @return Returns a new object.
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<is_pointer_type<T>::value && !is_map<T>::value && !is_multimap<T>::value && !has_begin_end<T>::value>::type* = nullptr>
        inline T Deserialize(const std::string &json)
        {
            static_assert(std::is_class<typename pointer_type<T>::type>::value, "Please use structs or objects!");

            ParseObject(json);

            T ret = CreatePointer<T>();
            ret->Deserialize(*this);
            m_Values.clear();

            return ret;
        }

        /**
         * @brief Deserializes a json object to a object.
         * 
         * To deserialize a json object you need to implement a Deserialize method, also you need a standard constructor.
         * @code
         *      class YourClass
         *      {
         *          public:
         *              YourClass() {} //<- Necessary
         * 
         *              ...
         *              void Deserialize(CJSON &json)
         *              {
         *                  ...
         *                  m_YourAttr = json.GetValue<std::string>("yourAttr");   //Gets the value of a json key.
         *                  ...
         *              }
         *              ...
         *      };
         * @endcode
         * 
         * @param json: JSON for deserialization.
         * @note This method returns only objects.
         * 
         * @return Returns a new object.
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && !is_map<T>::value && !is_multimap<T>::value && !has_begin_end<T>::value>::type* = nullptr>
        inline T Deserialize(const std::string &json)
        {
            static_assert(std::is_class<T>::value, "Please use structs or objects!");
            ParseObject(json);

            T ret;
            ret.Deserialize(*this);
            m_Values.clear();

            return ret;
        }

        /**
         * @brief Deserializes to map types.
         * 
         * @param json: JSON for deserialization.
         * @return Returns a new map type.
         * 
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && is_map<T>::value>::type* = nullptr>
        inline T Deserialize(const std::string &json)
        {
            static_assert(std::is_class<T>::value, "Please use structs or objects!");
            ParseObject(json);

            T Ret;
            for (auto &&e : m_Values)
                Ret[e.first] = ParseValue<typename T::mapped_type>(e.second);

            m_Values.clear();

            return Ret;
        }

        /**
         * @brief Deserializes to multimap types.
         * 
         * @param json: JSON for deserialization.
         * @return Returns a new multimap type.
         * 
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && is_multimap<T>::value>::type* = nullptr>
        inline T Deserialize(const std::string &json)
        {
            static_assert(std::is_class<T>::value, "Please use structs or objects!");
            ParseObject(json);

            T Ret;
            for (auto &&e : m_Values)
            {
                std::vector<typename T::mapped_type> arr = ParseValue<std::vector<typename T::mapped_type>>(e.second);
                for(auto &&ae : arr)
                    Ret.insert(std::pair<typename T::key_type, typename T::mapped_type>(e.first, ae));
            }

            m_Values.clear();

            return Ret;
        }

        /**
         * @brief Deserializes to all STL containers, which implements the push_back() or push_front() method.
         * 
         * @param json: JSON for deserialization.
         * @note Theoretically you can deserialize any container which implements these methods.
         * @return Returns a new container.
         * 
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && !is_map<T>::value && !is_multimap<T>::value && has_begin_end<T>::value>::type* = nullptr>
        inline T Deserialize(const std::string &json)
        {
            static_assert(std::is_class<T>::value, "Please use structs or objects!");
            return ParseValue<T>(json);
        }

        /**
         * @brief Parses a given json object.
         * 
         * @param obj: JSON object string.
         * @note You can access each value via @see GetValue.
         * 
         * @throw CJSONException If any error occurres.
         */
        inline void ParseObject(const std::string &obj)
        {
            m_Values.clear();
            std::string Key, Value;
            bool ValAllowed = false;
            bool KeyValueValid = false;
            bool ValidObject = false;

            /*
                Why we use raw pointers instead of iterators or an for loop?
                Because I think its much faster to use pointer arithmetic instead
                of iterator arithmetic or for loops.

                If I'm wrong with my thoughts, please correct me.
             */
            const char *beg = obj.c_str();
            const char *end = beg + obj.size();

            while (beg != end && *beg != '}')
            {
                switch (*beg)
                {
                    // Parses a string.
                    case '\"':
                    {
                        // Throw an exception if we found a string outside of an object.
                        if(!ValidObject)
                            throw CJSONException(JSONErrorType::INVALID_JSON_OBJECT);
                        else if(KeyValueValid)
                            throw CJSONException("Missing seperator ','.", JSONErrorType::MISSING_SEPERATOR);

                        // Quick and dirty check if we need a key or a value.
                        if(Key.empty())
                            Key = ParseString(beg, end);
                        else if(Value.empty() && ValAllowed)
                        {
                            ValAllowed = false;
                            KeyValueValid = true;
                            Value = ParseString(beg, end);
                        }
                        else   // Throw an exception if we doesn't expected a key or value. That occurs always if a colon is missing.
                            throw CJSONException("Missing seperator after '" + Key + "'", JSONErrorType::MISSING_SEPERATOR);                            
                    }break;

                    // Ignore all whitespace characters, which are outside of values.
                    case '\t':
                    case '\n':
                    case '\f':
                    case '\r':
                    case ' ':
                    {

                    }break;

                    // Parses an object or array.
                    case '{':
                    case '[':
                    {
                        // If this is the root object then set the validator flag.
                        if(!ValidObject)
                        {
                            ValidObject = true;
                            break;
                        }

                        // Throws an exception either if the key is missing or a value wasn't expected.
                        if(Key.empty())
                            throw CJSONException(JSONErrorType::MISSING_KEY); 
                        else if(!ValAllowed)
                            throw CJSONException("Missing seperator after '" + Key + "'", JSONErrorType::MISSING_SEPERATOR);  

                        ValAllowed = false;
                        KeyValueValid = true;

                        // Objects and arrays are saved as strings and parsed later.
                        if(*beg == '{')
                            Value = ParseBracketStr('{', '}', beg, end);
                        else if(*beg == '[')
                            Value = ParseBracketStr('[', ']', beg, end);
                    }break;

                    // Allows the next key value pair.
                    case ',': 
                    {
                        if(!ValidObject)
                            throw CJSONException(JSONErrorType::INVALID_JSON_OBJECT);

                        if(!KeyValueValid)
                            throw CJSONException("Lonly ','", JSONErrorType::WRONG_PLACED_SEPERATOR);        

                        KeyValueValid = false;
                    }break;

                    // Allows a value.
                    case ':':
                    {
                        if(!ValidObject)
                            throw CJSONException(JSONErrorType::INVALID_JSON_OBJECT);

                        if(Key.empty())
                            throw CJSONException(JSONErrorType::MISSING_KEY);

                        ValAllowed = true;
                    }break;

                    default:
                    {
                        if(!ValidObject)
                            throw CJSONException(JSONErrorType::INVALID_JSON_OBJECT);

                        if(Key.empty())
                            throw CJSONException(JSONErrorType::MISSING_KEY); 
                        else if(!ValAllowed)
                            throw CJSONException("Missing seperator after '" + Key + "'", JSONErrorType::MISSING_SEPERATOR);  

                        ValAllowed = false;
                        KeyValueValid = true;

                        switch (*beg)
                        {
                            // If a non escaped string begins with t or f it's maybe a boolean.
                            case 't':
                            case 'f':
                            {
                                Value = ParseBool(beg, end);
                            }break;

                            // If a non escaped string begins with n it's maybe a null object.
                            case 'n':
                            {
                                Value = ParseNull(beg, end);
                            }break;

                            default:
                            {
                                // If a non escaped string begins with a number, plus or minus it's maybe a number.
                                if(isalnum(*beg) || *beg == '-' || *beg == '+')
                                    Value = ParseNum(beg, end);
                                else    // Otherwise throw an exception.
                                    throw CJSONException("Invalid type for key '" + Key + "'", JSONErrorType::INVALID_TYPE);
                            }break;
                        }
                    }break;
                }

                // If key and value is set save it for later use.
                if(KeyValueValid)
                {
                    ValAllowed = false;
                    m_Values[Key] = Value;
                    Key.clear();
                    Value.clear();
                }

                if(beg != end)
                    beg++;
            }

            // Validates the object.
            if(ValidObject && *beg != '}')
                throw CJSONException(JSONErrorType::INVALID_JSON_OBJECT);
        }

        /**
         * @brief Adds a new value to the json.
         * 
         * @param Name: Name of the value in the json file.
         * @param val: Value of the json value.
         * @param IsObjectStr: Value is already an json object.
         * 
         * @note You can add any type. It can be primitive, pointer, object or container types. The object type need to implement the Serialize method.
         * @throw CJSONException If any error occurres.
         */
        template<class T>
        inline void AddPair(const std::string &Name, const T &val)
        {
            if(m_Values.find(Name) != m_Values.end())
                throw CJSONException("Name '" + Name + "' already exists!", JSONErrorType::NAME_ALREADY_EXITS);

            m_Values[Name] = ValueToString(val); 
        }

        /**
         * @brief Adds a new value to the json.
         * 
         * @param Name: Name of the value in the json file.
         * @param val: Value of the json value.
         * 
         * @note You can add any type. It can be primitive, pointer, object or container types. The object type need to implement the Serialize method.
         * @throw CJSONException If any error occurres.
         */
        inline void AddJSON(const std::string &Name, const std::string &val)
        {
            if(m_Values.find(Name) != m_Values.end())
                throw CJSONException("Name '" + Name + "' already exists!", JSONErrorType::NAME_ALREADY_EXITS);

            m_Values[Name] = val; 
        }

        /**
         * @brief Adds a new primitive array to the json.
         * 
         * @param Name: Name of the array in the json file.
         * @param val: Value of the json array.
         * @param Size: Size of the array.
         * 
         * @note You can add any type. It can be primitive or object array types. The object type need to implement the Serialize method.
         * @throw CJSONException If any error occurres.
         */
        template<class T>
        inline void AddPair(const std::string &Name, const T &val, size_t Size)
        {
            if(m_Values.find(Name) != m_Values.end())
                throw CJSONException("Name '" + Name + "' already exists!", JSONErrorType::NAME_ALREADY_EXITS);         

            m_Values[Name] = '[' + ArrayToStr(val, Size) + ']'; 
        }

        /**
         * @return Gets the type of the value.
         */
        inline JsonType GetType(const std::string &Key)
        {
            auto IT = m_Values.find(Key);
            if(IT == m_Values.end())
                throw CJSONException("Name '" + Key + "' not found", JSONErrorType::NAME_NOT_FOUND); 

            if(IT->second == "null")
                return JsonType::NIL;
            else if(IT->second == "true" || IT->second == "false")
                return JsonType::BOOLEAN;
            else if(IT->second[0] == '{') //Quick and dirty check.
                return JsonType::OBJECT;
            else if(IT->second[0] == '[') //Quick and dirty check.
                return JsonType::ARRAY;
            else if(IT->second[0] == '\"') //Quick and dirty check.
                return JsonType::STRING;
            
            if(IT->second.find("."))
            {
                try
                {
                    std::stod(IT->second); //Quick and dirty check.
                    return JsonType::DOUBLE;
                }
                catch(const std::exception& e)
                {
                    return JsonType::UNKNOWN;
                }
            }
            else
            {
                try
                {
                    std::stoi(IT->second); //Quick and dirty check.
                    return JsonType::INTEGER;
                }
                catch(const std::exception& e)
                {
                    return JsonType::UNKNOWN;
                }
            }
        }

        /**
         * @return Returns true if the given key exists.
         */
        inline bool HasKey(const std::string &Key)
        {
            return m_Values.find(Key) != m_Values.end();
        }

        /**
         * @brief Gets the value for a given name.
         * 
         * @param Name: Name of the value.
         * @param Default: Default value if the given value doesn't exists.
         * 
         * @note For primitive, pointer, std::string and container types.
         * 
         * @return Returns the requested value.
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<!std::is_class<typename std::remove_pointer<T>::type>::value || std::is_same<T, std::string>::value || has_push_back<T>::value>::type* = nullptr>
        T GetValue(const std::string &Name, const T &Default = T())
        {
            std::map<std::string, std::string>::iterator IT = m_Values.find(Name);
            if(IT == m_Values.end())
                return Default;

            if(IT->second == "null")
                return AddNullObj<T>();

            return ParseValue<T>(IT->second);
        }

        /**
         * @brief Gets the value for a given name.
         * 
         * @param Name: Name of the value.
         * 
         * @note For objects.
         * 
         * @return Returns the requested object.
         * @throw CJSONException If any error occurres.
         */
        template<class T, typename std::enable_if<std::is_class<typename pointer_type<T>::type>::value && !std::is_same<T, std::string>::value && !has_push_back<T>::value>::type* = nullptr>
        T GetValue(const std::string &Name)
        {
            std::map<std::string, std::string>::iterator IT = m_Values.find(Name);
            if(IT == m_Values.end())
                throw CJSONException("Name '" + Name + "' not found", JSONErrorType::NAME_NOT_FOUND); 

            if(IT->second == "null")
                return AddNullObj<T>();

            CJSON json;
            return json.Deserialize<T>(IT->second);
        }

        /**
         * @brief Gets the value for a given array.
         * 
         * @param Name: Name of the array.
         * @param[out] Size: The size of the array.
         * 
         * @return Returns a primitive array.
         * @throw CJSONException If any error occurres.
         */
        template<class T>
        T GetValue(const std::string &Name, size_t &Size)
        {
            std::map<std::string, std::string>::iterator IT = m_Values.find(Name);
            if(IT == m_Values.end())
            {
                Size = 0;
                return nullptr;
            }

            if(IT->second == "null")
            {
                Size = 0;
                return AddNullObj<T>();
            }

            return ParseArray<T>(IT->second, Size);
        }


        ~CJSON() {}

    private:
        std::map<std::string, std::string> m_Values;

        /**-----------------------------------------Blackmagic for SFINAE-----------------------------------------**/

        /** Creates a std::shared_ptr **/
        template<class T, typename std::enable_if<is_shared_ptr<T>::value>::type* = nullptr>
        inline T CreatePointer()
        {
            return T(new typename pointer_type<T>::type());
        }

        /** Creates a primitive pointer **/
        template<class T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
        inline T CreatePointer()
        {
            return new typename pointer_type<T>::type();
        }

        // Validates the json bool.
        inline std::string ParseBool(const char *&beg, const char *&end)
        {
            std::string Ret;

            while (beg != end)
            {
                if(*beg == ',' || *beg == '}' || *beg == ']')
                    break;

                Ret += *beg;
                beg++;
            } 

            beg--;

            if(Ret != "true" && Ret != "false")
                throw CJSONException("Expected bool", JSONErrorType::EXPECTED_BOOL);

            return Ret;
        }

        // Validates the json null object.
        inline std::string ParseNull(const char *&beg, const char *&end)
        {
            std::string Ret;

            while (beg != end)
            {
                if(*beg == ',' || *beg == '}' || *beg == ']')
                    break;

                Ret += *beg;

                beg++;
            } 

            beg--;

            if(Ret != "null")
                throw CJSONException("Expected null", JSONErrorType::EXPECTED_NULL);

            return Ret;
        }

        // Validates the json number.
        inline std::string ParseNum(const char *&beg, const char *&end)
        {
            std::string Ret;

            while (beg != end)
            {
                if(*beg == ',' || *beg == '}' || *beg == ']')
                    break;
                Ret += *beg;

                beg++;
            } 

            beg--;

            try
            {
                std::stod(Ret);
            }
            catch(const std::exception& e)
            {
                throw CJSONException("Expected number", JSONErrorType::EXPECTED_NUM);
            }

            return Ret;
        }

        /**
         * Groups all characters from OpenBracket to CloseBracket into a single string.
         * This is used for objects and arrays.
         */
        inline std::string ParseBracketStr(char OpenBracket, char CloseBracket, const char *&beg, const char *&end)
        {
            std::string Ret;
            int OpenBrackets = 0;

            while (beg != end)
            {
                Ret += *beg;

                // Counts the numbers of found open brackets. This is used to determine the end of the string.
                if(*beg == OpenBracket)
                    OpenBrackets++;
                else if(*beg == CloseBracket)
                    OpenBrackets--;

                if(OpenBrackets == 0)
                    break;

                beg++;
            } 

            return Ret;
        }

        // Parses a json string and return the unescaped string.
        inline std::string ParseString(const char *&beg, const char *&end)
        {
            std::string Ret;
            bool EscapeFound = false;

            beg++;
            while (beg != end)
            {
                if(*beg == '\"' && !EscapeFound)
                    break;

                Ret += *beg;

                if(*beg == '\\')
                    EscapeFound = true;
                else if(EscapeFound && *beg != '\\')
                    EscapeFound = false;

                beg++;
            }

            return UnescapeString(Ret);
        }

        inline int IsUTF8(unsigned char c)
        {
            if((c & 0xF0) == 0xF0)
                return 3;
            if((c & 0xE0) == 0xE0)
                return 2;
            if((c & 0xC0) == 0xC0)
                return 1;

            return 0;
        }

        inline int UTF8BitShift(unsigned char c)
        {
            if((c & 0xF0) == 0xF0)
                return 5;
            if((c & 0xE0) == 0xE0)
                return 4;
            if((c & 0xC0) == 0xC0)
                return 3;
            if((c & 0x80) == 0x80)
                return 2;

            return 0;
        }

        inline uint8_t UTF8Header(uint32_t UTF32)
        {
            if(UTF32 >= 0x80 && UTF32 <= 0x7FF)
                return 0xC0;
            if(UTF32 >= 0x800 && UTF32 <= 0xFFFF)
                return 0xE0;
            if(UTF32 >= 0x10000 && UTF32 <= 0x10FFFF)
                return 0xF0;

            return 0;
        }

        inline std::string NumToHexStr(uint32_t Num)
        {
            size_t Len = sizeof(uint32_t) + 1;
            char *Buf = new char[Len];
            Len = snprintf(Buf, Len, "%0*x", Len - 1, Num);

            return std::string(Buf, Buf + Len);
        }

        // Escapes a string.
        inline std::string EscapeString(std::string str)
        {
            str = ReplaceAll(str, "\\", "\\\\");
            str = ReplaceAll(str, "\"", "\\\"");
            str = ReplaceAll(str, "/", "\\/");
            str = ReplaceAll(str, "\b", "\\b");
            str = ReplaceAll(str, "\n", "\\n");
            str = ReplaceAll(str, "\f", "\\f");
            str = ReplaceAll(str, "\r", "\\r");
            str = ReplaceAll(str, "\t", "\\t");

            //UTF-8 decoding.
            for(size_t i = 0; i < str.size(); i++)
            {
                int Bytes = IsUTF8(str[i]);
                if(Bytes)
                {
                    int FreeBits = UTF8BitShift(str[i]);
                    uint8_t Buf[sizeof(uint32_t)] = {0};
                    Buf[Bytes] = str[i] & (0xFF >> FreeBits);

                    size_t Counter = 0;

                    for (; Counter < Bytes; Counter++)
                    {
                        size_t Pos = i + Counter + 1;
                        if(Pos >= str.size())   //End of string?
                            break;

                        int ShiftBits = UTF8BitShift(str[Pos]);
                        if(ShiftBits != 2)  //All second Bytes needs to begin with 10xxxxxx, otherwise its not utf-8.
                            break;

                        uint8_t c = str[Pos] & (0xFF >> ShiftBits);
                        Buf[Bytes - (Counter + 1)] = c;
                    }

                    //We have valid UTF8
                    if(Counter == Bytes)
                    {
                        uint32_t UTF32 = 0;
                        uint8_t Shift = 6;

                        for(char i = 0; i < Bytes; i++)
                        {
                            //ä - xxx00011 xx100100
                            //𝄞 - xxxxx000 xx011101 xx000100 xx011110

                            //1 - << 6
                            //ä - xxx000xx 11100100
                            //xxx000xx >> 2 - xxxxx000 11100100
                            //End



                            //1 - << 6
                            //𝄞 - xxxxx000 xx011101 xx0001xx 00011110
                            //xx0001xx >> 2 - xxxxx000 xx011101 xxxx0001 00011110

                            //2 - << 4
                            //𝄞 - xxxxx000 xx01xxxx 11010001 00011110
                            //xx01xxxx >> 4 - xxxxx000 xxxxxx01 11010001 00011110

                            //3 - << 2
                            //𝄞 - xxxxxxxx xxx00001 11010001 00011110
                            //End

                            //i = xx100100

                            uint8_t Carry = (Buf[i + 1] & ((0x4 << i * 2) - 1));
        
                            UTF32 |= (Buf[i] | (Carry << Shift)) << (8 * i);
                            Buf[i + 1] >>= i * 2 + 2;

                            Shift -= 2;

                            // UTF32 |= (Buf[i] | ((Buf[i + 1] & ((0x4 << i * 2) - 1)) << ((Bytes + 1 - i) * 2))) << (8 * i);
                            // Buf[i + 1] >>= i * 2 + 2;
                        }

                        UTF32 |= Buf[Bytes] << (8 * Bytes);

                        std::string Encode = "\\u" + NumToHexStr(UTF32);
                        str.erase(i, Bytes + 1);
                        str.insert(i, Encode);
                        i += Encode.size() - 1;
                    }
                }
            }

            return str;
        }

        inline uint32_t ExtractUTF32(const std::string &str, size_t Pos, int &Len)
        {
            std::string Buf;

            for (Len = 0; Len < sizeof(uint32_t); Len++)
            {
                if(!isxdigit(str[Pos]))
                    break;

                Buf = Buf + str[Pos];
                Pos++;
            }

            return (uint32_t)std::stoi(Buf, nullptr, 16);
        }

        // Unescapes a json string.
        inline std::string UnescapeString(std::string str)
        {
            str = ReplaceAll(str, "\\\\", "\\");
            str = ReplaceAll(str, "\\\"", "\"");
            str = ReplaceAll(str, "\\/", "/");
            str = ReplaceAll(str, "\\b", "\b");
            str = ReplaceAll(str, "\\n", "\n");
            str = ReplaceAll(str, "\\f", "\f");
            str = ReplaceAll(str, "\\r", "\r");
            str = ReplaceAll(str, "\\t", "\t");

            //UTF-8 encoding.
            size_t Pos = -1;
            do
            {
                Pos = str.find("\\u", Pos + 1);
                if(Pos != std::string::npos)
                {
                    int Len = 0;
                    uint32_t UTF32 = ExtractUTF32(str, Pos + 2, Len);
                    uint8_t Header = UTF8Header(UTF32);
                    uint8_t *UTF32Ptr = (uint8_t*)&UTF32;
                    int Bytes = IsUTF8(Header);
                    uint8_t Carry = 0;

                    if(Bytes)
                    {
                        for (int i = 0; i < Bytes + 1; i++)
                        {
                            uint8_t OldCarry = Carry; 
                            uint8_t OldShift = 8 - (i * 2 + 2);
                            uint8_t Shift = (i - 1) * 2 + 2;

                            Carry = (UTF32Ptr[i] & ((0x4 << i * 2) - 1) << OldShift) >> OldShift;
                            UTF32Ptr[i] = ((UTF32Ptr[i] << Shift) & 0x3F) | OldCarry | ((i == Bytes) ? Header : 0x80);
                        }
                    }

                    int Pos1 = Pos;

                    str.erase(Pos, Len + 2);
                    for (int i = Bytes + 1; i--;)
                    {
                        if(Pos1 >= str.size())
                            str += UTF32Ptr[i];
                        else
                            str.insert(Pos1, std::string(1, UTF32Ptr[i]));

                        Pos1++;
                    }
                }
            }while (Pos != std::string::npos);

            return str;
        }

        // Helper function to replace all substrings.
        inline std::string ReplaceAll(std::string str, const std::string &Old, const std::string &New)
        {
            size_t Pos = 0;
            while (Pos != std::string::npos)
            {
                Pos = str.find(Old, Pos);

                if(Pos != std::string::npos)
                {
                    str = str.erase(Pos, Old.size());
                    str.insert(Pos, New);
                    Pos += New.size();
                }
            }

            return str;            
        }

        // Converts all integral and floating point values, except bool, to string.
        template<class T, typename std::enable_if<(std::is_integral<T>::value || std::is_floating_point<T>::value) && !std::is_same<T, bool>::value>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            return std::to_string(val);
        }

        // Converts a bool value to string.
        template<class T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            return val ? "true" : "false"; 
        }

        // Converts a nullptr to string.
        template<class T, typename std::enable_if<std::is_null_pointer<T>::value>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            return "null"; 
        }

        // Converts a string or cstring to json string.
        template<class T, typename std::enable_if<std::is_same<T, std::string>::value || (is_pointer_type<T>::value && std::is_same<typename std::remove_const<typename std::remove_pointer<T>::type>::type, char>::value)>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            return '"' + EscapeString(val) + '"'; 
        }

        // Converts a object to string.
        template<class T, typename std::enable_if<std::is_class<typename pointer_type<T>::type>::value && !std::is_same<T, std::string>::value && !has_begin_end<T>::value>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            CJSON json;
            return json.Serialize(val); 
        }

        // Converts a non map container to a json array string.
        template<class T, typename std::enable_if<!is_map<T>::value && !is_multimap<T>::value && has_begin_end<T>::value && !std::is_same<T, std::string>::value>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            std::string Ret;
            typename T::const_iterator beg = val.begin();
            typename T::const_iterator end = val.end();

            while (beg != end)
            {
                if(!Ret.empty())
                    Ret += ',';

                Ret += ValueToString(*beg);
                beg++;
            }
            
            return '[' + Ret + ']';
        }

        // Converts a map type to a json object string.
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && is_map<T>::value>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            static_assert(std::is_class<typename std::remove_pointer<T>::type>::value, "Please use structs or objects!");
            std::string Ret;

            for (auto &&e : val)
            {
                if(!Ret.empty())
                    Ret += ',';

                Ret += '"' + e.first + "\":" + ValueToString(e.second);
            }

            return '{' + Ret + '}';
        }

        // Converts a multimap type to a json object with arrays string. 
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && is_multimap<T>::value>::type* = nullptr>
        inline std::string ValueToString(const T &val)
        {
            static_assert(std::is_class<typename std::remove_pointer<T>::type>::value, "Please use structs or objects!");
            std::string Ret;
            std::string CurKey;
            std::string CurValues;

            for (auto &&e : val)
            {
                if(CurKey != e.first)
                {
                    if(CurKey != "")
                    {
                        if(!Ret.empty())
                            Ret += ",";

                        Ret += '"' + CurKey + "\":[" + CurValues + ']';
                        CurValues.clear();
                    }

                    CurKey = e.first;
                }

                if(!CurValues.empty())
                    CurValues += ',';

                CurValues += ValueToString(e.second);
            }

            if(CurKey != "")
            {
                if(!Ret.empty())
                    Ret += ",";

                Ret += '"' + CurKey + "\":[" + CurValues + ']';
                CurValues.clear();
            }

            return '{' + Ret + '}';
        }

        // Converts a primitive array to string.
        template<class T, typename std::enable_if<!std::is_class<typename pointer_type<T>::type>::value>::type* = nullptr>
        inline std::string ArrayToStr(const T &arr, size_t Size)
        {
            std::string ret;

            for (size_t i = 0; i < Size; i++)
            {
                if(!ret.empty())
                    ret += ',';

                ret += std::to_string((&*arr)[i]);
            }

            return ret;
        }

        // Converts a primitive array with objects to string.
        template<class T, typename std::enable_if<std::is_class<typename pointer_type<T>::type>::value>::type* = nullptr>
        inline std::string ArrayToStr(const T &arr, size_t Size)
        {
            std::string ret;
            CJSON json;

            for (size_t i = 0; i < Size; i++)
            {
                if(!ret.empty())
                    ret += ',';

                ret += json.Serialize((&*arr)[i]);
            }

            return ret;
        }

        // Converts a string to an integral.
        template<class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value && !is_pointer_type<T>::value>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            long ret = 0;

            try
            {
                ret = std::stol(val);
            }
            catch(const std::exception& e)
            {
                throw CJSONException("", JSONErrorType::INVALID_CAST);
            }

            return (T)ret;
        }

        // Converts a string to a floating point.
        template<class T, typename std::enable_if<std::is_floating_point<T>::value && !std::is_same<T, bool>::value && !is_pointer_type<T>::value>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            double ret = 0;

            try
            {
                ret = std::stod(val);
            }
            catch(const std::exception& e)
            {
                throw CJSONException("", JSONErrorType::INVALID_CAST);
            }

            return (T)ret;
        }

        // Converts a string to boolean.
        template<class T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            bool ret = false;

            if(val == "true")
                ret = true;
            else if(val != "false")
                throw CJSONException("Invalid cast", JSONErrorType::INVALID_CAST);

            return ret;
        }

        // Called if a invalid cast occured
        template<class T, typename std::enable_if<(std::is_class<T>::value && !std::is_same<T, std::string>::value && !has_push_back<T>::value && !has_push_front<T>::value && !(is_map<T>::value || is_multimap<T>::value)) /*|| std::is_pointer<T>::value*/>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            throw CJSONException(JSONErrorType::INVALID_CAST);
        }

        //Called for maps
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && (is_map<T>::value || is_multimap<T>::value)>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            CJSON json;
            return json.Deserialize<T>(val);
        }

        // Converts a string to string. 
        template<class T, typename std::enable_if<std::is_same<T, std::string>::value>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            return val; 
        }
        
        // Converts a string to cstring. 
        template<class T, typename std::enable_if<is_pointer_type<T>::value && std::is_same<typename pointer_type<T>::type, char>::value>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            T Ret = new typename pointer_type<T>::type[val.size() + 1];
            strcpy(Ret, val.c_str());
            Ret[val.size()] = '\0';

            return Ret;
        }

        // Deserializes an object inside an array and add it to a container type.
        template<class Container, typename std::enable_if<std::is_class<typename std::remove_pointer<typename Container::value_type>::type>::value && !std::is_same<typename Container::value_type, std::string>::value && !has_push_back<typename Container::value_type>::value>::type * = nullptr>
        inline void DeserializeObject(Container &con, const std::string &val)
        {
            CJSON json;
            Push(con, json.Deserialize<typename Container::value_type>(val));
        }

        //For Strings
        template<class Container, typename std::enable_if<std::is_same<typename Container::value_type, std::string>::value || has_push_back<typename Container::value_type>::value>::type * = nullptr>
        inline void DeserializeObject(Container &con, const std::string &val)
        {
            Push(con, val);
        }

        // Called if an array of objects is passed to a container type with primitive type.
        template<class Container, typename std::enable_if<(!std::is_class<typename std::remove_pointer<typename Container::value_type>::type>::value || has_push_back<typename Container::value_type>::value) && !std::is_same<typename Container::value_type, std::string>::value>::type * = nullptr>
        inline void DeserializeObject(Container &con, const std::string &val)
        {
            throw CJSONException("Can't cast object to none object type", JSONErrorType::INVALID_CAST);
        }

        // Helper function for container types with push_back() methods.
        template<class T, class V, typename std::enable_if<has_push_back<T>::value>::type* = nullptr>
        inline void Push(T &ret, const V &val)
        {
            ret.push_back(val);
        }

        // Helper function for container types with push_front() methods.
        template<class T, class V, typename std::enable_if<!has_push_back<T>::value && has_push_front<T>::value>::type* = nullptr>
        inline void Push(T &ret, const V &val)
        {
            ret.push_front(val);
        }

        template<class T, typename std::enable_if<is_pointer_type<T>::value>::type* = nullptr>
        inline T AddNullObj()
        {
            return nullptr;
        }

        template<class T, typename std::enable_if<!is_pointer_type<T>::value>::type* = nullptr>
        inline T AddNullObj()
        {
            return T();
        }

        // Parses a container type.
        template<class T, typename std::enable_if<!is_pointer_type<T>::value && (has_push_back<T>::value || has_push_front<T>::value) && !std::is_same<T, std::string>::value>::type* = nullptr>
        inline T ParseValue(const std::string &val)
        {
            using ValType = typename T::value_type;

            T Ret;

            bool ValAllowed = true;
            bool ValidArray = false;

            const char *beg = val.c_str();
            const char *end = beg + val.size();

            while (beg != end && *beg != ']')
            {
                switch (*beg)
                {
                    case '\"':
                    {
                        if(!ValidArray)
                            throw CJSONException(JSONErrorType::INVALID_ARRAY);

                        if(ValAllowed)
                        {
                            ValAllowed = false;
                            Push(Ret, ParseValue<ValType>(ParseString(beg, end)));
                        }
                        else
                            throw CJSONException("Missing seperator after value", JSONErrorType::MISSING_SEPERATOR);                            
                    }break;

                    case '\t':
                    case '\n':
                    case '\f':
                    case '\r':
                    case ' ':
                    {

                    }break;

                    case '{':
                    case '[':
                    {
                        if(!ValidArray)
                        {
                            ValidArray = true;
                            break;
                        }

                        if(!ValAllowed)
                            throw CJSONException("Missing seperator after value", JSONErrorType::MISSING_SEPERATOR);  

                        ValAllowed = false;

                        CJSON json;

                        if(*beg == '{')
                            DeserializeObject(Ret, ParseBracketStr('{', '}', beg, end));
                        else if(*beg == '[')
                            Push(Ret, ParseValue<ValType>(ParseBracketStr('[', ']', beg, end)));
                    }break;

                    case ',': 
                    {
                        if(!ValidArray)
                            throw CJSONException(JSONErrorType::INVALID_ARRAY);

                        ValAllowed = true;
                    }break;

                    default:
                    {
                        if(!ValidArray)
                            throw CJSONException(JSONErrorType::INVALID_ARRAY);

                        if(!ValAllowed)
                            throw CJSONException("Missing seperator after value", JSONErrorType::MISSING_SEPERATOR); 

                        ValAllowed = false;

                        switch (*beg)
                        {
                            case 't':
                            case 'f':
                            {
                                Push(Ret, ParseValue<ValType>(ParseBool(beg, end)));
                            }break;

                            case 'n':
                            {
                                ParseNull(beg, end);
                                Push(Ret, AddNullObj<ValType>());
                            }break;

                            default:
                            {
                                if(isalnum(*beg) || *beg == '-' || *beg == '+')
                                    Push(Ret, ParseValue<ValType>(ParseNum(beg, end)));
                                else
                                    throw CJSONException(JSONErrorType::INVALID_TYPE);
                            }break;
                        }
                    }break;
                }

                if(beg != end)
                    beg++;
            }

            if(ValidArray && *beg != ']')
                throw CJSONException(JSONErrorType::INVALID_ARRAY);

            return Ret;
        }

        // Creates a primitive array inside a std::shared_ptr.
        template<class T, typename std::enable_if<is_shared_ptr<T>::value>::type* = nullptr>
        T CreateArray(size_t Size)
        {
            return T(new typename pointer_type<T>::type[Size], std::default_delete<typename pointer_type<T>::type[]>());
        }

        // Creates a primitive array.
        template<class T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
        T CreateArray(size_t Size)
        {
            return new typename pointer_type<T>::type[Size];
        }

        // Parses an array of primitive types.
        template<class T, typename std::enable_if<!std::is_class<typename pointer_type<T>::type>::value>::type* = nullptr>
        inline T ParseArray(const std::string &val, size_t &Size)
        {
            using ArrType = typename pointer_type<T>::type;
            std::vector<ArrType> Arr = ParseValue<std::vector<ArrType>>(val);

            Size = Arr.size();
            T ret = CreateArray<T>(Size);

            memcpy(&*ret, Arr.data(), Size * sizeof(ArrType));

            return ret;
        }

        // Parses an array of object types.
        template<class T, typename std::enable_if<std::is_class<typename pointer_type<T>::type>::value>::type* = nullptr>
        inline T ParseArray(const std::string &val, size_t &Size)
        {
            using ArrType = typename pointer_type<T>::type;
            std::vector<ArrType> Arr = ParseValue<std::vector<ArrType>>(val);

            Size = Arr.size();
            T ret = CreateArray<T>(Size);

            for (size_t i = Size; i--;)
                (&*ret)[i] = Arr[i];

            return ret;
        }

        /**-----------------------------------------Blackmagic for SFINAE-----------------------------------------**/
};

#endif //JSON_HPP