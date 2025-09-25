# class *Intrinsic* from sys.core.lang

A collection of miscellaneous intrinsic operations and helper functions.

## Methods

### MemCopy

```C#
static def MemCopy(destination: Ptr<Byte>, source: Ptr<Byte>, bytes: PtrSize);
```

#### Brief

Does a C standard library `memcpy` operation, copying a number of bytes from a source to a destination.

#### Parameters
> *destination* => the destination pointer  
> *source* => the source pointer  
> *bytes* => the number of bytes to copy  
***

### CStrLen

```C#
static def CStrLen(source: Ptr<Byte>): PtrSize;
```

#### Brief
Does a C standard library `strlen` operation, retuning the length of a null terminated C string.

Should not be used on `sys.core.lang.String` since it can contain valid null characters.

#### Parameters
> *source* => the C string  
#### Returns
> the length
***

### CpuID

```C#
static def CpuID(ref cpu: CArray<Int>, val level: Int);
```

#### Brief
Executes a CPU identifying set of instructions.

#### Parameters
> *cpu* => the output table  
> *level* => the level of the requested information  
***

### Min

```C#
Min(a: T, b: T): T;
```

#### Brief
Returns the minimum between two values.
***

### Max

```C#
Max(a: T, b: T): T;
```

#### Brief
Returns the maximum between two values.
***

### Clamped

```C#
Clamped(a: T, min: T, max: T): T;
```

#### Brief
Returns the `a` parameter clamped between two values.
***

### Clamp

```C#
Clamp(ref a: T, min: T, max: T);
```

#### Brief
Clamps the `a` parameter between two values.
***

