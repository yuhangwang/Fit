Function Adaptor
----------------

A function adaptor takes a function(or functions) and returns a new function with enhanced capability. Each adaptor has a functional form with a corresponding class with `_adaptor` appended to it:

```cpp
template<class... Fs>
FunctionAdaptor_adaptor<Fs...> FunctionAdaptor(Fs...);
```

Both the functional form and the class form can be used to construct the adaptor.

Static Function Adaptor
-----------------------

A static function adaptor is a function adaptor that doesn't have a functinal form. It is only a class. It has an additional requirement that the function is `DefaultConstructible`:

```cpp
template<class... Fs>
class StaticFunctionAdaptor;
```

Decorator
---------

A decorator is a function that returns a function adaptor. The function adaptor may be an unspecified 0r private type.

```cpp
template<class... Ts>
FunctionAdaptor Decorator(Ts...);
```