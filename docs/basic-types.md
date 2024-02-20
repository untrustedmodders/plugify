# Basic Type mapping

The following lists how the types are exposed to the C++ API.

| Type                    | Alias      | Ref ? |
|-------------------------|------------|-------|
| void                    | void       | false |
| bool                    | bool       | true  |
| char                    | char8      | true  |
| char16_t                | char16     | true  |
| int8_t                  | int8       | true  |
| int16_t                 | int16      | true  |
| int32_t                 | int32      | true  |
| int64_t                 | int64      | true  |
| uint8_t                 | uint8      | true  |
| uint16_t                | uint16     | true  |
| uint32_t                | uint32     | true  |
| uint64_t                | uint64     | true  |
| uintptr_t               | ptr64      | true  |
| float                   | float      | true  |
| double                  | double     | true  |
| void*                   | function   | false |
| std::string             | string     | true  |
| std::vector<bool>       | arraybool  | true  |
| std::vector<char>       | arraychar8 | true  |
| std::vector<char16_t>   | arraychar16| true  |
| std::vector<int8_t>     | arrayint8  | true  |
| std::vector<int16_t>    | arrayint16 | true  |
| std::vector<int32_t>    | arrayint32 | true  |
| std::vector<int64_t>    | arrayint64 | true  |
| std::vector<uint8_t>    | arrayuint8 | true  |
| std::vector<uint16_t>   | arrayuint16| true  |
| std::vector<uint32_t>   | arrayuint32| true  |
| std::vector<uint64_t>   | arrayuint64| true  |
| std::vector<uintptr_t>  | arrayptr64 | true  |
| std::vector<float>      | arrayfloat | true  |
| std::vector<double>     | arraydouble| true  |
| std::vector<std::string>| arraystring| true  |

## Exported Functions

### Example 1

- **Function Name:** Example_Function
- **Exported Method Name:** Example_Function_Exported_Name
- **Parameters:**
    - Parameter 1:
        - **Type:** ptr64
        - **Name:** param1
    - Parameter 2:
        - **Type:** string
        - **Name:** param2
    - Parameter 3:
        - **Type:** int32
        - **Name:** param3
    - Parameter 4:
        - **Type:** arrayint8
        - **Name:** param4
        - **Reference:** true
- **Return Type:** string

Here's an example template that combines these elements:

```json
{
  "name": "Example_Function",
  "funcName": "Example_Function_Exported_Name",
  "paramTypes": [
    {
      "type": "ptr64",
      "name": "param1"
    },
    {
      "type": "string",
      "name": "param2"
    },
    {
      "type": "int32",
      "name": "param3"
    },
    {
      "type": "arrayint8",
      "name": "param4",
      "ref": true
    }
  ],
  "retType": {
    "type": "string"
  }
}
```

How it will look like on C++ side:

```c++
extern "C" void Example_Function(std::string& ret, void* p1, const std::string& p2, int32_t p3, std::vector<uint8_t>& p4)
```

### Example 2

- **Function Name:** Example_Function
- **Exported Method Name:** Example_Function_Exported_Name
- **Parameters:**
    - Parameter 1:
        - **Type:** float
        - **Name:** param1
    - Parameter 2:
        - **Type:** double
        - **Name:** param2
        - **Reference:** true
    - Parameter 3:
        - **Type:** function
        - **Name:** param3
- **Return Type:** void

Here's an example template that combines these elements:

```json
{
  "name": "Example_Function",
  "funcName": "Example_Function_Exported_Name",
  "paramTypes": [
    {
      "type": "float",
      "name": "param1"
    },
    {
      "type": "double",
      "name": "param2",
      "ref": "true"
    },
    {
      "type": "function",
      "name": "param3",
      "prototype":
      {
        "name": "Example_Callback_Function",
        "funcName": "Example_Callback_Function_Exported_Name",
        "paramTypes": [
          {
            "type": "ptr64",
            "name": "param1"
          },
          {
            "type": "string",
            "name": "param2"
          },
          {
            "type": "int32",
            "name": "param3"
          }
        ],
        "retType": {
          "type": "string"
        }
      }
    }
  ],
  "retType": {
    "type": "void"
  }
}
```

How it will look like on C++ side:

```c++
using Example_Callback_Function = std::string(*)(void*, const std::string&, int32_t);
extern "C" void Example_Function(float p1, double& p2, Example_Callback_Function p4)
```

---

### Pointers
	For ref and out paramaters you'll use the corresponding
	reference type. With setting "ref" parameter to true.
	So if you have a type listed as "int32", you should use
	"int32_t&" in your implementation. 

### Arrays and Strings
	Arrays of any type must be described with a std::vector<>& and the
	strings should be pass by std::string&. 

### Return
	In x86-x64 calling conventions, which determine how function arguments are passed. 
	For pod structures or objects returned by value, the caller must allocate memory 
	for the return value and pass a pointer to it as the first argument.
	