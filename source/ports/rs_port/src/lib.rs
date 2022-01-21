/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

use std::ffi::CString;
use std::os::raw::{c_char, c_double, c_float, c_int, c_long, c_short, c_void};

pub use abi::interface as abi_interface;
pub use inline;

#[derive(Debug)]
pub struct Error(String);

/// Enum of all possible Metacall types to allow for safe conversion between them and c_types
#[derive(Debug)]
pub enum Any {
    Null,              // from c_null
    Short(i16),        // from c_short
    Int(i32),          // from c_int
    Long(i64),         // from c_long
    Float(f32),        // from c_float
    Double(f64),       // from c_double
    Bool(bool),        // from c_bool
    Char(char),        // from c_char
    Str(String),       // from *const u8 (null terminated)
    Array(Vec<Any>),   // from *mut *mut c_void
    Buffer(Vec<u8>),   // from *const u8 (non-null terminated) (raw binary data)
    Pointer(Box<Any>), // from *mut c_void
    Function(Box<fn(Any) -> Any>), // from a C function pointer
                       // METACALL_FUTURE
}

impl From<c_short> for Any {
    fn from(val: c_short) -> Self {
        Any::Short(val)
    }
}
impl From<c_int> for Any {
    fn from(val: c_int) -> Self {
        Any::Int(val)
    }
}
impl From<c_long> for Any {
    fn from(val: c_long) -> Self {
        Any::Long(val)
    }
}
impl From<c_char> for Any {
    fn from(val: c_char) -> Self {
        Any::Char(val as u8 as char)
    }
}
impl From<bool> for Any {
    fn from(val: bool) -> Self {
        Any::Bool(val)
    }
}
impl From<c_float> for Any {
    fn from(val: c_float) -> Self {
        Any::Float(val)
    }
}
impl From<c_double> for Any {
    fn from(val: c_double) -> Self {
        Any::Double(val)
    }
}

pub fn initialize() -> Result<(), &'static str> {
    if unsafe { abi_interface::metacall_initialize() } != 0 {
        Err("Metacall failed to initialize")
    } else {
        Ok(())
    }
}

pub fn load_from_file(
    tag: &str,
    scripts: impl IntoIterator<Item = impl AsRef<str>>,
) -> Result<(), &'static str> {
    // allocate a safe C String
    let c_tag = CString::new(tag).expect("Conversion to C String failed");

    let owned_scripts: Vec<_> = scripts
        .into_iter()
        .map(|x| CString::new(x.as_ref()).expect("Conversion to C String failed"))
        .collect();

    let mut ref_c_scripts: Vec<_> = owned_scripts
        .iter()
        .map(|s| s.as_ptr())
        .map(|p| p as *const u8)
        .collect();

    if unsafe {
        abi_interface::metacall_load_from_file(
            c_tag.as_ptr(),
            ref_c_scripts.as_mut_ptr(),
            ref_c_scripts.len(),
            std::ptr::null_mut(),
        )
    } != 0
    {
        return Err("MetaCall failed to load script from file");
    }

    Ok(())
}

pub fn load_from_memory(
    tag: &str,
    script: String,
) -> Result<(), &'static str> {
    let c_tag = CString::new(tag).expect("Conversion to C String failed");
    let script_len = script.len();
    let c_script = CString::new(script).expect("Conversion to C String failed");

    if unsafe {
        abi_interface::metacall_load_from_memory(
            c_tag.as_ptr(),
            c_script.as_ptr(),
            script_len,
            std::ptr::null_mut(),
        )
    } != 0
    {
        return Err("MetaCall failed to load script from memory");
    }

    Ok(())
}

// Possible types as variants in Rust
pub fn metacall<'a>(
    func: &str,
    args: impl IntoIterator<Item = &'a Any>,
) -> Result<Any, &'static str> {
    let c_function = CString::new(func).expect("Conversion to C String failed");
    let c_func: *mut c_void = unsafe { abi_interface::metacall_function(c_function.as_ptr()) };

    if c_func.is_null() {
        return Err("Function Not Found");
    }

    let mut c_args: Vec<*mut c_void> = args
        .into_iter()
        .map(|arg| unsafe {
            match arg {
                Any::Short(x) => abi_interface::metacall_value_create_short(*x),
                Any::Int(x) => abi_interface::metacall_value_create_int(*x),
                Any::Long(x) => abi_interface::metacall_value_create_long(*x),
                Any::Float(x) => abi_interface::metacall_value_create_float(*x),
                Any::Double(x) => abi_interface::metacall_value_create_double(*x),
                Any::Bool(x) => abi_interface::metacall_value_create_bool(*x as c_int),
                Any::Char(x) => abi_interface::metacall_value_create_char(*x as c_char),
                Any::Str(x) => {
                    let st = CString::new(x.as_str()).expect("can't convert to c str");

                    abi_interface::metacall_value_create_string(st.as_ptr(), x.len())
                }
                _ => todo!(),
            }
        })
        .collect();

    let ret: *mut c_void =
        unsafe { abi_interface::metacallfv_s(c_func, c_args.as_mut_ptr(), c_args.len()) };

    let mut rt = Any::Null;

    if !ret.is_null() {
        /* TODO: This should be done by an enum or something mimicking the enum in metacall.h */
        unsafe {
            match abi_interface::metacall_value_id(ret) {
                0 => {
                    rt = Any::Bool(abi_interface::metacall_value_to_bool(ret) != 0);
                }
                1 => {
                    rt = Any::Char(abi_interface::metacall_value_to_char(ret) as u8 as char);
                }
                2 => {
                    rt = Any::Short(abi_interface::metacall_value_to_short(ret));
                }
                3 => {
                    rt = Any::Int(abi_interface::metacall_value_to_int(ret));
                }
                4 => {
                    rt = Any::Long(abi_interface::metacall_value_to_long(ret));
                }
                5 => {
                    rt = Any::Float(abi_interface::metacall_value_to_float(ret));
                }
                6 => {
                    rt = Any::Double(abi_interface::metacall_value_to_double(ret));
                }
                7 => {
                    let st = std::ffi::CStr::from_ptr(abi_interface::metacall_value_to_string(ret));

                    rt = Any::Str(String::from(
                        st.to_str().expect("couldn't convert CStr to &str"),
                    ));
                }
                8 => {
                    // METACALL_BUFFER
                }
                9 => {
                    // METACALL_ARRAY
                }
                10 => {
                    // METACALL_MAP
                }
                11 => {
                    // METACALL_PTR
                }
                12 => {
                    // METACALL_FUTURE
                }
                13 => {
                    // METACALL_FUNCTION
                }
                14 => {
                    rt = Any::Null;
                }
                _ => {}
            }
            abi_interface::metacall_value_destroy(ret);
        }
    }
    for arg in c_args {
        unsafe {
            abi_interface::metacall_value_destroy(arg);
        }
    }
    Ok(rt)
}

pub fn destroy() {
    unsafe {
        abi_interface::metacall_destroy();
    }
}

/// Doc test to check if the code can build an run
#[cfg(test)]
mod tests {
    struct Defer<F: FnOnce()>(Option<F>);

    impl<F: FnOnce()> Drop for Defer<F> {
        fn drop(&mut self) {
            if let Some(f) = self.0.take() {
                f()
            }
        }
    }

    /// Defer execution of a closure until the constructed value is dropped
    /// Works at the end of the scope or manual drop() function
    pub fn defer<F: FnOnce()>(f: F) -> impl Drop {
        Defer(Some(f))
    }

    #[test]
    fn test_defer() {
        use std::cell::RefCell;

        let i = RefCell::new(0);

        {
            let _d = defer(|| *i.borrow_mut() += 1);

            assert_eq!(*i.borrow(), 0);
        }

        assert_eq!(*i.borrow(), 1);
    }

    #[test]
    fn test_metacall() {
        let _d = defer(crate::destroy);

        match crate::initialize() {
            Err(e) => {
                println!("{}", e);
                panic!();
            }
            _ => println!(" Hello World Metacall created "),
        }

        let scripts = ["test.mock"];

        if let Err(e) = crate::load_from_file("mock", &scripts) {
            println!("{}", e);
            panic!();
        }

        match crate::metacall("new_args", &[crate::Any::Str("a".to_string())]) {
            Ok(ret) => match ret {
                crate::Any::Str(value) => {
                    assert_eq!("Hello World".to_string(), value);

                    println!("Result: {}", value);
                }
                _ => {
                    assert_eq!(0, 1);

                    panic!();
                }
            },
            Err(e) => {
                println!("{}", e);

                assert_eq!(0, 1);

                panic!();
            }
        }

        if let Err(e) = crate::load_from_memory("py", "def pyfn():\n\treturn 23".to_string()) {
            println!("{}", e);
            panic!();
        }

        match crate::metacall("pyfn", &[]) {
            Ok(ret) => match ret {
                crate::Any::Long(value) => {
                    assert_eq!(23, value);

                    println!("Result: {}", value);
                }
                _ => {
                    assert_eq!(0, 1);

                    panic!();
                }
            },
            Err(e) => {
                println!("{}", e);

                assert_eq!(0, 1);

                panic!();
            }
        }
    }
}
