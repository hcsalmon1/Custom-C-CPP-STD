

template <typename F>
class Defer {
public:
	Defer(F&& f) : func(std::forward<F>(f)), active(true) {}
	~Defer() {
		if (active) func();
	}

	// Disable copy
	Defer(const Defer&) = delete;
	Defer& operator=(const Defer&) = delete;

	// Enable move
	Defer(Defer&& other) noexcept
		: func(std::move(other.func)), active(other.active) {
		other.active = false;
	}

private:
	F func;
	bool active;
};

template <typename F>
Defer<F> make_defer(F&& f) {
	return Defer<F>(std::forward<F>(f));
}

#pragma region Error Macros

///if err is an error, return
#define TRY(x)  \
	if ((x).isError()) {                \
        return;                   \
    }

///if Result is an error, print and return
#define TRY_PRINT(x)  \
	if ((x).isError()) {                \
		(x).printError();			  \
        return;                   \
    }

/// if err is an error, return value
#define TRY_RETURN(x, val)  \
    if ((x).isError()) {                \
        return val;                   \
    }

/// if err is an error, return value
#define TRY_PRINT_RETURN(x, val)  \
    if ((x).isError()) {                \
        return val;                   \
    }

#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)

#define DEFER(code) \
    auto CONCAT(_defer_, __LINE__) = make_defer([&]() { code; })

#pragma endregion

using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;

struct String;
template <typename T>
struct Slice;

#pragma region Errors

enum class ErrorCode {
	None,
	Allocation_Failed,
	Allocation_Out_Of_Stack_Memory,
	Allocation_Resize_Not_Supported_On_Stack_Memory,
	Allocation_Double_Free,
	Allocation_Use_After_Free,
	Allocation_Zero_Initial_Capacity,
	Allocation_Empty_Slice,
	String_Char_Pointer_Over_Max,
	String_Null_Data,
	String_Length_Is_Zero,
	String_No_Null_Terminator,
	String_Format_Failed,
	Slice_Null_Data,
	Slice_Copy_Source_Is_Null,
	Slice_Copy_Length_Too_Small,
	Slice_Copy_Source_Length_Zero,
	DynamicBuffer_Null_Data,
	DynamicBuffer_Null_Slice_Data,
	DynamicBuffer_Index_Out_Of_Bounds,
	File_Open_Failed,
	File_Close_Failed,
	File_Read_Failed,
	File_Write_Failed,
	File_Is_Null,
};

const char* errorToString(ErrorCode code) {
	switch (code) {

	case ErrorCode::None:
		return "None";

	case ErrorCode::Allocation_Failed:
		return "Allocation_Failed";
	case ErrorCode::Allocation_Out_Of_Stack_Memory:
		return "Allocation_Out_Of_Stack_Memory";
	case ErrorCode::Allocation_Resize_Not_Supported_On_Stack_Memory:
		return "Allocation_Resize_Not_Supported_On_Stack_Memory";
	case ErrorCode::Allocation_Double_Free:
		return "Allocation_Double_Free";
	case ErrorCode::Allocation_Use_After_Free:
		return "Allocation_Use_After_Free";
	case ErrorCode::Allocation_Zero_Initial_Capacity:
		return "Allocation_Zero_Initial_Capacity";
	case ErrorCode::Allocation_Empty_Slice:
		return "Allocation_Empty_Slice";

	case ErrorCode::String_Char_Pointer_Over_Max:
		return "String_Char_Pointer_Over_Max";
	case ErrorCode::String_Null_Data:
		return "String_Null_Data";
	case ErrorCode::String_Length_Is_Zero:
		return "String_Length_Is_Zero";
	case ErrorCode::String_No_Null_Terminator:
		return "String_No_Null_Terminator";
	case ErrorCode::String_Format_Failed:
		return "String_Format_Failed";

	case ErrorCode::Slice_Null_Data:
		return "Slice_Null_Data";
	case ErrorCode::Slice_Copy_Source_Is_Null:
		return "Slice_Copy_Source_Is_Null";
	case ErrorCode::Slice_Copy_Length_Too_Small:
		return "Slice_Copy_Length_Too_Small";
	case ErrorCode::Slice_Copy_Source_Length_Zero:
		return "Slice_Copy_Source_Length_Zero";

	case ErrorCode::DynamicBuffer_Null_Data:
		return "DynamicBuffer_Null_Data";
	case ErrorCode::DynamicBuffer_Null_Slice_Data:
		return "DynamicBuffer_Null_Slice_Data";
	case ErrorCode::DynamicBuffer_Index_Out_Of_Bounds:
		return "DynamicBuffer_Index_Out_Of_Bounds";

	case ErrorCode::File_Open_Failed:
		return "File_Open_Failed";
	case ErrorCode::File_Close_Failed:
		return "File_Close_Failed";
	case ErrorCode::File_Read_Failed:
		return "File_Read_Failed";
	case ErrorCode::File_Write_Failed:
		return "File_Write_Failed";
	case ErrorCode::File_Is_Null:
		return "File_Is_Null";

	default:
		return "Unknown";
	}
}

struct Error {

	ErrorCode code;

	bool isError() {
		return code != ErrorCode::None;
	}
	void reset() {
		code = ErrorCode::None;
	}
	void printError() {
		fprintf(stderr, "Error: %s\n", errorToString(code));
	}
	void setError(ErrorCode _code) {
		code = _code;
	}
	void orPanic() {
		if (isError()) {
			abort();
		}
	}
	const char* toString() {
		return errorToString(code);
	}

	static Error none() {
		return Error{
			.code = ErrorCode::None,
		};
	}
	static Error init() {
		return Error{
			.code = ErrorCode::None,
		};
	}
	static Error create(ErrorCode code) {
		return Error{
			.code = code
		};
	}

};

template <typename T>
struct Result {
	T value;
	Error err;

	bool isError() {
		return err.isError();
	}
	void printError() {
		err.printError();
	}
	T unwrapOrPanic() {
		if (isError()) {
			printError();
			abort();
		}
		return value;
	}
	ErrorCode getCode() {
		return err.code;
	}
	Error toError() {
		return Error::create(err.code);
	}
};

template <typename T>
static Result<T> errorResult(ErrorCode code) {
	Result<T> result;
	result.err = Error{ .code = code };
	return result;
}

template <typename T>
static Result<T> dataResult(T data) {
	Result<T> result;
	result.value = data;
	result.err = Error{ .code = ErrorCode::None };
	return result;
}

/// <summary>
/// Has a value or doesn't. 
/// Null = has_value == false
/// 
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
struct Optional {
	T value;
	bool has_value;

	bool isNull() {
		return has_value == false;
	}
	bool notNull() {
		return has_value == true;
	}
	T unwrapOrPanic() {
		if (isNull()) {
			fprintf(stderr, "Error: tried to unwrap a null value\n");
			abort();
		}
		return value;
	}
};

template <typename T>
static Optional<T> nullOptional() {
	Optional<T> optional;
	optional.has_value = false;
	return optional;
}

template <typename T>
static Optional<T> valueOptional(T value) {
	Optional<T> optional;
	optional.value = value;
	optional.has_value = true;
	return optional;
}

#pragma endregion

#pragma region Allocations

struct Allocator {
	void* context;

	void(*alloc)(Error* err, void* context, size_t size, size_t align, void** out_ptr);

	void(*free)(void* context, void* ptr, size_t size);

	void(*allocSlice)(Error* err, void* context, size_t elem_size, size_t count, size_t align, void** out_ptr);

	void(*freeSlice)(void* context, void* ptr, size_t elem_size, size_t count);

	void(*resizeSlice)(Error* err, void* context, void* ptr, size_t elem_size, size_t old_count, size_t new_count, size_t align, void** out_ptr);
};

/// <summary>
/// Pointer and a length
/// </summary>
template <typename T>
struct Slice {
	T* data;
	size_t length;

	bool isNotNull() {
		return data != NULL;
	}
	bool isNull() {
		return data == NULL;
	}
	static Slice null() {
		return {
			.data = NULL,
			.length = 0
		};
	}
	static Slice create(T* data, const size_t size) {
		return {
			.data = data,
			.length = size,
		};
	}

	static Slice createConst(const T* data, const size_t size) {
		return {
			.data = data,
			.length = size,
		};
	}

	static Slice<char> createChar(const char* data) {
		const size_t length = strlen(data);
		return {
			.data = (char*)data,
			.length = length,
		};
	}

	void copySlice(Error* err, Slice<T> source) {

		if (length < source.length) {
			err->setError(ErrorCode::Slice_Copy_Length_Too_Small);
			return;
		}
		if (data == NULL) {
			err->setError(ErrorCode::Slice_Null_Data);
			return;
		}
		if (source.data == NULL) {
			err->setError(ErrorCode::Slice_Copy_Source_Is_Null);
			return;
		}

		for (size_t i = 0; i < source.length; i++) {
			data[i] = source.data[i];
		}
	}
};

template <typename T>
Slice<T> allocSlice(Error* err, Allocator allocator, size_t count);

/// <summary>
/// Frees a slice using the given allocator
/// Slice must be a pointer because it sets data to null to stop potential double frees.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="allocator"></param>
/// <param name="slice"></param>
/// <returns></returns>
template <typename T>
void freeSlice(Allocator allocator, Slice<T>* slice) {
	if (slice->data == NULL) {
		return;
	}
	allocator.freeSlice(allocator.context, (void*)slice->data, sizeof(T), slice->length);
	slice->data = NULL;
}

void freeStringSlice(Allocator allocator, Slice<const char>* slice) {
	if (slice->data == NULL) {
		return;
	}
	allocator.freeSlice(allocator.context, (void*)slice->data, sizeof(char), slice->length + 1);
	slice->data = NULL;
}


struct HeapAllocator {

	static void alloc(Error* err, void*, size_t size, size_t, void** out_ptr) {

		void* data = malloc(size);
		if (data == NULL) {
			err->setError(ErrorCode::Allocation_Failed);
			return;
		}
		*out_ptr = data;
	}

	static void free(void*, void* ptr, size_t size) {
		::free(ptr);
	}

	static void allocSlice(Error* err, void*, size_t elem, size_t count, size_t, void** out_ptr) {

		if (count == 0) {
			err->code = ErrorCode::Allocation_Zero_Initial_Capacity;
			return;
		}
		void* data = malloc(elem * count);
		if (data == NULL) {
			err->code = ErrorCode::Allocation_Failed;
			return;
		}
		*out_ptr = data;
	}

	static void freeSlice(void*, void* ptr, size_t, size_t) {
		::free(ptr);
	}

	static void resizeSlice(Error* err, void*, void* ptr, size_t elem, size_t, size_t new_count, size_t, void** out_ptr) {

		void* data = realloc(ptr, elem * new_count);
		if (data == NULL) {
			err->setError(ErrorCode::Allocation_Failed);
			return;
		}
		*out_ptr = data;
	}

	static Allocator init() {
		return {
			NULL,
			&alloc,
			&free,
			&allocSlice,
			&freeSlice,
			&resizeSlice
		};
	}
};

static inline size_t alignUp(size_t ptr, size_t align) {
	return (ptr + align - 1) & ~(align - 1);
}

static size_t alignForward(size_t ptr, size_t align) {
	size_t modulo = ptr & (align - 1);
	if (modulo != 0) {
		ptr += (align - modulo);
	}
	return ptr;
}

struct StackAllocator {

	struct Context {
		uint8_t* buffer;
		size_t capacity;
		size_t offset;
	};

	static void alloc(Error* err, void* context, size_t size, size_t align, void** out_ptr) {
		Context* allocator = (Context*)context;

		size_t aligned = alignUp(allocator->offset, align);
		size_t end = aligned + size;

		if (end > allocator->capacity) {
			err->setError(ErrorCode::Allocation_Out_Of_Stack_Memory);
			return;
		}

		*out_ptr = allocator->buffer + aligned;
		allocator->offset = end;
	}

	static void free(void*, void*, size_t size) {
		
	}

	static void allocSlice(Error* err, void* ctx, size_t elem_size, size_t count, size_t align, void** out_ptr) {
		return alloc(err, ctx, elem_size * count, align, out_ptr);
	}

	static void freeSlice(void*, void*, size_t, size_t) {

	}

	static void resizeSlice(Error* err, void*, void*, size_t, size_t, size_t, size_t, void**) {
		err->setError(ErrorCode::Allocation_Resize_Not_Supported_On_Stack_Memory);
	}

	static Allocator init(Slice<u8> buffer) {
		Context* context = (Context*)buffer.data;

		*context = {
			.buffer = buffer.data + sizeof(Context),
			.capacity = buffer.length - sizeof(Context),
			.offset = 0
		};

		return {
			context,
			&alloc,
			&free,
			&allocSlice,
			&freeSlice,
			&resizeSlice
		};
	}
};

struct TrackingAllocatorContext {
	Allocator backing;

	size_t alloc_count;
	size_t free_count;

	size_t bytes_allocated;
	size_t bytes_freed;

	size_t live_bytes;
	size_t peak_bytes;
};

struct TrackingAllocator {

	static void printData(void* context) {

		TrackingAllocatorContext* ctx = (TrackingAllocatorContext*)context;

		printf("---- Tracking Allocator Stats ----\n");
		printf("Alloc calls   : %zu\n", ctx->alloc_count);
		printf("Free calls    : %zu\n", ctx->free_count);
		printf("Bytes alloc   : %zu\n", ctx->bytes_allocated);
		printf("Bytes freed   : %zu\n", ctx->bytes_freed);
		printf("Live bytes    : %zu\n", ctx->live_bytes);
		printf("Peak bytes    : %zu\n", ctx->peak_bytes);

		printf("----------------------------------\n");
	}




	static void trackingAlloc(Error* err, void* context, size_t size, size_t align, void** out_ptr) {

		TrackingAllocatorContext* ctx = (TrackingAllocatorContext*)context;

		ctx->backing.alloc(err, ctx->backing.context, size, align, out_ptr);
		TRY(*err);

		ctx->alloc_count++;
		ctx->bytes_allocated += size;
		ctx->live_bytes += size;
		if (ctx->live_bytes > ctx->peak_bytes) {
			ctx->peak_bytes = ctx->live_bytes;
		}
	}

	static void trackingResizeSlice(Error* err, void* context, void* ptr, size_t elem, size_t old_count, size_t new_count, size_t align, void** out_ptr) {

		TrackingAllocatorContext* ctx = (TrackingAllocatorContext*)context;

		size_t old_size = elem * old_count;
		size_t new_size = elem * new_count;

		ctx->backing.resizeSlice(err, ctx->backing.context, ptr, elem, old_count, new_count, align, out_ptr);
		TRY(*err);

		if (new_size > old_size) {
			size_t delta = new_size - old_size;
			ctx->bytes_allocated += delta;
			ctx->live_bytes += delta;
		}
		else {
			size_t delta = old_size - new_size;
			ctx->bytes_freed += delta;
			ctx->live_bytes -= delta;
		}

		if (ctx->live_bytes > ctx->peak_bytes) {
			ctx->peak_bytes = ctx->live_bytes;
		}
	}

	static void trackingFreeSlice(void* context, void* ptr, size_t elem, size_t count) {

		TrackingAllocatorContext* ctx = (TrackingAllocatorContext*)context;
		size_t size = elem * count;

		ctx->free_count++;
		ctx->bytes_freed += size;
		ctx->live_bytes -= size;

		return ctx->backing.freeSlice(ctx->backing.context, ptr, elem, count);
	}

	static void trackingAllocSlice(Error* err, void* context, size_t elem, size_t count, size_t align, void** out_ptr) {

		TrackingAllocatorContext* ctx = (TrackingAllocatorContext*)context;
		size_t size = elem * count;

		ctx->backing.allocSlice(err, ctx->backing.context, elem, count, align, out_ptr);
		TRY(*err);

		ctx->alloc_count++;
		ctx->bytes_allocated += size;
		ctx->live_bytes += size;
		if (ctx->live_bytes > ctx->peak_bytes) {
			ctx->peak_bytes = ctx->live_bytes;
		}
	}

	static void trackingFree(void* context, void* ptr, size_t size) {
		TrackingAllocatorContext* ctx = (TrackingAllocatorContext*)context;

		ctx->free_count++;
		ctx->bytes_freed += size;
		ctx->live_bytes -= size;

		ctx->backing.free(ctx->backing.context, ptr, size);
	}

	static Allocator init(Allocator backing, TrackingAllocatorContext* ctx) {
		*ctx = {};
		ctx->backing = backing;

		return {
			ctx,
			&trackingAlloc,
			&trackingFree,
			&trackingAllocSlice,
			&trackingFreeSlice,
			&trackingResizeSlice
		};
	}
};

struct Arena {
	u8* buffer;
	size_t capacity;
	size_t offset;
};

struct FixedBufferArenaAllocator {
	
	static void arenaAlloc(Error* err, void* context, size_t size, size_t align, void** out_ptr) {

		Arena* arena = (Arena*)context;

		size_t current_ptr = (size_t)(arena->buffer + arena->offset);
		size_t aligned_ptr = alignForward(current_ptr, align);
		size_t new_offset = (aligned_ptr - (size_t)arena->buffer) + size;

		if (new_offset > arena->capacity) {
			err->setError(ErrorCode::Allocation_Failed);
			return;
		}

		arena->offset = new_offset;
		*out_ptr = (void*)aligned_ptr;
	}

	static void arenaAllocSlice(Error* err, void* context, size_t elem_size, size_t count, size_t align, void** out_ptr) {
		if (count == 0) {
			err->setError(ErrorCode::Allocation_Zero_Initial_Capacity);
			return;
		}

		arenaAlloc(err, context, elem_size * count, align, out_ptr);
	}

	static void arenaFree(void*, void*, size_t) {}
	static void arenaFreeSlice(void*, void*, size_t, size_t) {}

	static void arenaResizeSlice(Error* err, void* context, void* ptr, size_t elem_size, size_t old_count, size_t new_count, size_t align, void** out_ptr) { 

		Arena* arena = (Arena*)context;

		size_t old_size = elem_size * old_count;
		size_t new_size = elem_size * new_count;

		uint8_t* expected_end = (uint8_t*)ptr + old_size;

		uint8_t* arena_end = arena->buffer + arena->offset;

		if (expected_end == arena_end) {
			// It was the last allocation — can grow/shrink
			size_t new_offset = arena->offset - old_size + new_size;

			if (new_offset > arena->capacity) {
				err->setError(ErrorCode::Allocation_Failed);
				return;
			}

			arena->offset = new_offset;
			*out_ptr = ptr;
			return;
		}

		// Otherwise allocate new memory and copy
		void* new_mem;
		arenaAlloc(err, context, new_size, align, &new_mem);
		if (err->isError()) {
			return;
		}

		memcpy(new_mem, ptr, old_size < new_size ? old_size : new_size);
		*out_ptr = new_mem;
	}

	static void arenaReset(Arena* arena) {
		arena->offset = 0;
	}

	static Allocator arenaInit(Arena* arena, void* buffer, size_t capacity) {
		arena->buffer = (uint8_t*)buffer;
		arena->capacity = capacity;
		arena->offset = 0;

		return {
			arena,
			&arenaAlloc,
			&arenaFree,
			&arenaAllocSlice,
			&arenaFreeSlice,
			&arenaResizeSlice
		};
	}
};

template <typename T>
T* alloc(Error* err, Allocator allocator) {
	T* ptr;
	allocator.alloc(err, allocator.context, sizeof(T), alignof(T), (void**)&ptr);
	return ptr;
}

template <typename T>
Slice<T> allocSlice(Error* err, Allocator allocator, size_t count) {
	T* ptr;
	allocator.allocSlice(err, allocator.context, sizeof(T), count, alignof(T), (void**)&ptr);
	TRY_RETURN(*err, Slice<T>::null());

	Slice<T> slice;
	slice.data = ptr;
	slice.length = count;
	return slice;
}

template <typename T>
void freePtr(Allocator allocator, T** ptr) {
	if (ptr == NULL) {
		return;
	}
	allocator.free(allocator.context, (void*)(*ptr), sizeof(T));
	*ptr = NULL;
}

#pragma endregion

Optional<size_t> getCharPointerLengthSafe(const char* input, const size_t max) {
	size_t index = 0;
	while (index < max) {
		char c = input[index];
		if (c == '\0') {
			return valueOptional<size_t>(index);
		}
		index += 1;
	}
	return nullOptional<size_t>();
}

struct CharSlice {
	char* data;
	size_t length;
};

struct CharSlices {
	Slice<Slice<char>> data;

	static CharSlices null() {
		return {
			.data = NULL,
		};
	}
	void free(Allocator allocator) {
		freeSlice(allocator, &data);
	}
};

struct Strings {

	static size_t countLines(Slice<char> input)
	{
		if (input.data == NULL || input.length == 0) {
			return 0;
		}

		size_t count = 0;

		for (size_t i = 0; i < input.length; ++i)
		{
			if (input.data[i] == '\n') {
				count++;
			}
		}

		// If last character is not '\n', we have one more line
		if (input.length > 0 && input.data[input.length - 1] != '\n') {
			count++;
		}

		return count;
	}

	static CharSlices toLines(Error* err, Allocator allocator, Slice<char> input) {

		if (input.data == NULL) {
			err->setError(ErrorCode::Slice_Null_Data);
			return CharSlices::null();
		}

		CharSlices null = CharSlices::null();

		size_t line_count = countLines(input);

		if (line_count == 0) {
			return null;
		}

		Slice<Slice<char>> buffer = allocSlice<Slice<char>>(err, allocator, line_count);
		TRY_RETURN(*err, null);

		size_t slice_index = 0;
		size_t start = 0;

		for (size_t i = 0; i < input.length; ++i) {

			if (input.data[i] == '\n') {

				size_t len = i - start;

				// handle CRLF
				if (len > 0 && input.data[i - 1] == '\r') {
					len--;
				}

				Slice<char> slice = Slice<char>::create(input.data + start, len);
				buffer.data[slice_index] = slice;
				slice_index++;

				start = i + 1;
			}
		}

		// Final line (if not newline-terminated)
		if (start < input.length) {
			buffer.data[slice_index] = Slice<char>::create(input.data + start, input.length - start);
			slice_index++;
		}

		CharSlices result;
		result.data = buffer;
		result.data.length = slice_index;

		return result;
	}

	static CharSlices split(Error* err, Slice<char> input, char delimiter) {
		if (input.data == NULL) {
			err->setError(ErrorCode::Slice_Null_Data);
			return CharSlices::null();
		}
		size_t count = 0;
		size_t start = 0;
		CharSlices output = {
			.data = NULL,
		};
		for (size_t i = 0; i < input.length; ++i) {

			if (input.data[i] == delimiter) {

				if (count >= output.data.length) {
					err->setError(ErrorCode::Slice_Copy_Length_Too_Small);
					return CharSlices::null();
				}

				output.data.data[count] = Slice<char>::create(
					input.data + start, i - start
				);
				count++;
				start = i + 1;
			}
		}
		// Final segment
		if (start <= input.length) {

			if (count >= output.data.length) {
				err->setError(ErrorCode::Slice_Copy_Length_Too_Small);
				return CharSlices::null();
			}

			output.data.data[count] = Slice<char>::create(
				input.data + start, input.length - start
			);
			count++;
		}
		output.data.length = count;
		return output;
	}
};


/// <summary>
/// An allocated pointer and a length.
/// Cannot be resized.
/// </summary>
struct String {
	char* data;
	size_t length;

	Slice<char> toCharSlice() const {
		return Slice<char> {
			.data = data,
			.length = length
		};
	}
	
	void printLine() const {
		if (data == NULL) {
			printf("Warning string data: NULL!!");
			return;
		}
		for (size_t i = 0; i < length; i++) {
			printf("%c", data[i]);
		}
		printf("\n");
	}
	
	void print() const {
		if (data == NULL) {
			printf("Warning string data: NULL!!");
			return;
		}
		for (size_t i = 0; i < length; i++) {
			printf("%c", data[i]);
		}
	}
	
	static void printSliceLine(Slice<void> input) {
		if (input.data == NULL) {
			printf("Warning string data: NULL!!");
			return;
		}
		Slice<char> slice = Slice<char>::create((char*)input.data, input.length);
		for (size_t i = 0; i < slice.length; i++) {
			printf("%c", slice.data[i]);
		}
		printf("\n");
	}

	static void printSlice(Slice<void> input) {
		if (input.data == NULL) {
			printf("Warning string data: NULL!!");
			return;
		}
		Slice<char> slice = Slice<char>::create((char*)input.data, input.length);
		for (size_t i = 0; i < slice.length; i++) {
			printf("%c", slice.data[i]);
		}
	}

	static void printSliceLine(Slice<char> input) {
		if (input.data == NULL) {
			printf("Warning string data: NULL!!");
			return;
		}
		for (size_t i = 0; i < input.length; i++) {
			printf("%c", input.data[i]);
		}
		printf("\n");
	}

	static void printSlice(Slice<char> input) {
		if (input.data == NULL) {
			printf("Warning string data: NULL!!");
			return;
		}
		for (size_t i = 0; i < input.length; i++) {
			printf("%c", input.data[i]);
		}
	}

	static String null() {
		return {
			.data = NULL,
			.length = 0,
		};
	}

	bool isNull() const {
		return data == NULL;
	}

	/// <summary>
	/// Gets the char at the given index
	/// if in range and data is not null
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	Optional<char> getChar(size_t index) const {
		if (index >= length) {
			return nullOptional<char>();
		}
		if (data == NULL) {
			return nullOptional<char>();
		}
		return valueOptional(data[index]);
	}

	const char* toConstCharPtr(Error* err) const {
		if (data == NULL) {
			err->setError(ErrorCode::String_Null_Data);
			return "";
		}
		if (length == 0) {
			return "";
		}
		return data;
	}

	bool compare(const String* input) const {
		if (length != input->length) {
			return false;
		}
		for (size_t i = 0; i < input->length; i++) {
			if (input->data[i] != data[i]) {
				return false;
			}
		}
		return true;
	}

	bool compare(const char* input, const size_t _length) const {
		if (length != _length) {
			return false;
		}
		for (size_t i = 0; i < length; i++) {
			if (input[i] != data[i]) {
				return false;
			}
		}
		return true;
	}

	bool contains(const char c) const {

		if (data == NULL) {
			return false;
		}

		for (size_t i = 0; i < length; i++) {
			if (c == data[i]) {
				return true;
			}
		}
		return false;
	}

	void free(Allocator allocator) {
		if (data == NULL) {
			return;
		}
		Slice<const char> slice = {
			.data = data,
			.length = length,
		};
		//freeSlice(allocator, &slice);
		freeStringSlice(allocator, &slice);
	}

	void toLower() {
		if (data == NULL) return;

		for (size_t i = 0; i < length; i++) {
			char c = data[i];

			// ASCII uppercase range
			if (c >= 'A' && c <= 'Z') {
				data[i] = c + 32;
			}
		}
	}

	void toHigher() {
		if (data == NULL) {
			return;
		}

		for (size_t i = 0; i < length; i++) {
			char c = data[i];

			// ASCII lowercase range
			if (c >= 'a' && c <= 'z') {
				data[i] = c - 32;
			}
		}
	}

	void reverse() {
		if (data == NULL) {
			return;
		}

		if (length <= 1) {
			return;
		}

		size_t left = 0;
		size_t right = length - 1;

		while (left < right) {
			char temp = data[left];
			data[left] = data[right];
			data[right] = temp;

			left++;
			right--;
		}
	}

	static String create(Error* err, Allocator allocator, const char* input, size_t max) {

		String null = String::null();

		if (input == NULL) {
			err->code = ErrorCode::String_Null_Data;
			return null;
		}

		Optional<size_t> input_length = getCharPointerLengthSafe(input, max);
		if (input_length.isNull()) {
			err->setError(ErrorCode::String_Char_Pointer_Over_Max);
			return null;
		}

		Slice<char> memory = allocSlice<char>(err, allocator, input_length.value + 1);
		TRY_RETURN(*err, null);

		for (size_t i = 0; i < input_length.value; i++) {
			memory.data[i] = input[i];
		}

		memory.data[input_length.value] = '\0';

		return {
			.data = memory.data,
			.length = input_length.value
		};
	}

	static String create(Error* err, Allocator allocator, Slice<char> input) {

		String null = String::null();

		if (input.data == NULL) {
			return null;
		}

		size_t visible_length = input.length;

		bool already_terminated =
				visible_length > 0 &&
				input.data[visible_length - 1] == '\0';

		size_t allocation_length =
				already_terminated ? 
					visible_length : visible_length + 1;

		Slice<char> memory = allocSlice<char>(err, allocator, allocation_length);
		TRY_RETURN(*err, null);

		// copy only visible characters
		for (size_t i = 0; i < visible_length; i++) {
			memory.data[i] = input.data[i];
		}

		// enforce terminator
		memory.data[allocation_length - 1] = '\0';
		return String{
			.data = memory.data,
			.length = visible_length,
		};
	}

	static String create(Error* err, Allocator allocator, char* input, size_t max) {
		if (input == NULL) {
			return String::null();
		}

		Optional<size_t> input_length = getCharPointerLengthSafe(input, max);
		if (input_length.isNull()) {
			err->setError(ErrorCode::String_Char_Pointer_Over_Max);
			return String::null();
		}

		Slice<char> memory = allocSlice<char>(err, allocator, input_length.value + 1);
		if (err->isError()) {
			return String::null();
		}

		for (size_t i = 0; i < input_length.value; i++) {
			memory.data[i] = input[i];
		}

		memory.data[input_length.value] = '\0';

		return String{
			.data = memory.data,
			.length = input_length.value
		};
	}

	CharSlices toLines(Error* err, Allocator allocator) {
		if (isNull()) {
			err->setError(ErrorCode::String_Null_Data);
			return CharSlices::null();
		}

		Slice<char> view = toCharSlice();

		return Strings::toLines(err, allocator, view);
	}

};

static String allocPrint(Error* err, Allocator allocator, const char* format, ...) {

	if (format == NULL) {
		err->setError(ErrorCode::String_Null_Data);
		return String::null(); 
	}

	va_list args;
	va_start(args, format);

	// Copy args because vsnprintf consumes them
	va_list args_copy;
	va_copy(args_copy, args);

	// Get required length (excluding null terminator)
	int required = vsnprintf(NULL, 0, format, args);
	va_end(args);

	if (required < 0) {
		va_end(args_copy);
		err->setError(ErrorCode::String_Format_Failed);
		return String::null();
	}

	size_t visible_length = (size_t)required;

	// Allocate +1 for null terminator
	Slice<char> memory = allocSlice<char>(err, allocator, visible_length + 1);
	if (err->isError()) {
		va_end(args_copy);
		return String::null();
	}

	// Write formatted string
	int written = vsnprintf(memory.data, visible_length + 1, format, args_copy);

	va_end(args_copy);

	if (written < 0) {
		err->setError(ErrorCode::String_Format_Failed);
		return String::null();
	}

	return String{
		.data = memory.data,
		.length = visible_length
	};
}

template<typename T>
struct DynamicBuffer {
	T* data;
	size_t length;
	size_t capacity;

	void ensureCapacity(Error* err, Allocator allocator, size_t needed) {

		if (needed <= capacity) {
			return;
		}

		size_t new_capacity = capacity * 2;
		if (new_capacity < needed) {
			new_capacity = needed;
		}

		void* new_data_void = NULL;

		allocator.resizeSlice(
			err,
			allocator.context,
			data,
			sizeof(T),
			capacity,
			new_capacity,
			alignof(T),
			&new_data_void
		);
		TRY(*err);

		data = (T*)new_data_void;
		capacity = new_capacity;

		return;
	}

	void append(Error* err, Allocator allocator, T value) {
		ensureCapacity(err, allocator, length + 1);
		TRY(*err);

		data[length] = value;
		length += 1;
	}

	void copySliceIntoEnd(Slice<T> slice) {
		for (size_t i = 0; i < slice.length; i++) {
			data[length + i] = slice.data[i];
		}
		length += slice.length;
	}

	void appendSlice(Error* err, Allocator allocator, Slice<T> slice) {

		if (data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}
		if (slice.length == 0) {
			return;
		}
		if (slice.data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}

		ensureCapacity(err, allocator, length + slice.length);
		if (err->isError()) {
			return;
		}

		copySliceIntoEnd(slice);
	}

	void freeDynBuffer(Allocator allocator) {

		if (data != NULL) {
			allocator.freeSlice(allocator.context, data, sizeof(T), capacity);
		}

		data = NULL;
		length = 0;
		capacity = 0;
	}

	void clear() {
		length = 0;
	}

	void removeAt(Error* err, const size_t index) {

		if (data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}

		if (index >= length) {
			err->setError(ErrorCode::DynamicBuffer_Index_Out_Of_Bounds);
			return;
		}

		for (size_t i = index; i < length - 1; i++) {
			data[i] = data[i + 1];
		}

		length -= 1;
	}

	void removeAtSwap(Error* err, size_t index) {

		if (data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}

		if (index >= length) {
			err->setError(ErrorCode::DynamicBuffer_Index_Out_Of_Bounds);
			return;
		}

		data[index] = data[length - 1];
		length -= 1;
	}

	static DynamicBuffer<T> null() {
		return {
			.data = NULL,
			.length = 0,
			.capacity = 0
		};
	}

	Slice<T> toSlice() {
		const Slice<T> slice = {
			.data = data,
			.length = length,
		};
		return slice;
	}

	Slice<T> toOwnedSlice(Error* err, Allocator allocator) {
		if (data == NULL || length == 0) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return Slice<T>::null();
		}
		Slice<T> new_slice = allocSlice<T>(err, allocator, length);
		if (err->isError()) {
			return Slice<T>::null();
		}

		memcpy(new_slice.data, data, length);
		return new_slice;
	}
};

template<typename T>
DynamicBuffer<T> createDynBuffer(Error* err, Allocator allocator, size_t initial_capacity) {

	DynamicBuffer<T> buf;
	DynamicBuffer<T> null = DynamicBuffer<T>::null();

	if (initial_capacity == 0) {
		err->setError(ErrorCode::Allocation_Zero_Initial_Capacity);
		return null;
	}

	Slice<T> data = allocSlice<T>(err, allocator, initial_capacity);
	TRY_RETURN(*err, null);

	buf.data = data.data;
	if (buf.data == NULL) {
		return null;
	}

	buf.length = 0;
	buf.capacity = initial_capacity;

	return buf;
}

struct StringBuilder {
	char* data;
	size_t length;
	size_t capacity;

	static StringBuilder create(Error* err, Allocator allocator, size_t initial_capacity) {

		StringBuilder sb;
		StringBuilder null = StringBuilder::null();

		if (initial_capacity == 0) {
			err->setError(ErrorCode::Allocation_Zero_Initial_Capacity);
			return null;
		}

		Slice<char> data_result = allocSlice<char>(err, allocator, initial_capacity);
		TRY_RETURN(*err, null);

		sb.data = data_result.data;
		sb.length = 0;
		sb.capacity = initial_capacity;

		return sb;
	}

	static StringBuilder null() {
		return {
			.data = NULL,
			.length = 0,
			.capacity = 0
		};
	}

	void ensureCapacity(Error* err, Allocator allocator, size_t needed) {

		if (needed <= capacity) {
			return;
		}

		size_t new_capacity = capacity * 2;
		if (new_capacity < needed) {
			new_capacity = needed;
		}

		void* new_data_void = NULL;

		allocator.resizeSlice(
			err,
			allocator.context,
			data,
			sizeof(char),
			capacity,
			new_capacity,
			alignof(char),
			&new_data_void
		);

		TRY(*err);

		data = (char*)new_data_void;
		capacity = new_capacity;
	}

	void append(Error* err, Allocator allocator, char value) {
		ensureCapacity(err, allocator, length + 1);
		TRY(*err);

		data[length] = value;
		length += 1;
	}

	void copySliceIntoEnd(Slice<const char> slice) {
		for (size_t i = 0; i < slice.length; i++) {
			if (slice.data[i] == '\0') {
				break;
			}
			data[length + i] = slice.data[i];
		}
		length += slice.length;
	}

	void copySliceIntoEnd(Slice<char> slice) {
		for (size_t i = 0; i < slice.length; i++) {
			if (slice.data[i] == '\0') {
				break;
			}
			data[length + i] = slice.data[i];
		}
		length += slice.length;
	}

	void copyStringIntoEnd(String string) {
		for (size_t i = 0; i < string.length; i++) {
			if (string.data[i] == '\0') {
				break;
			}
			data[length + i] = string.data[i];
		}
		length += string.length;
	}

	void appendSlice(Error* err, Allocator allocator, Slice<const char> slice) {
		if (data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
		}
		if (slice.length == 0) {
			return;
		}
		if (slice.data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
		}

		ensureCapacity(err, allocator, length + slice.length);
		TRY(*err);

		copySliceIntoEnd(slice);
	}

	void appendSlice(Error* err, Allocator allocator, Slice<char> slice) {
		if (data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}
		if (slice.length == 0) {
			return;
		}
		if (slice.data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}

		ensureCapacity(err, allocator, length + slice.length);
		TRY(*err);

		copySliceIntoEnd(slice);
	}

	void appendString(Error* err, Allocator allocator, String string) {

		if (data == NULL) {
			 Error::create(ErrorCode::DynamicBuffer_Null_Data);
			 return;
		}
		if (string.length == 0) {
			return;
		}
		if (string.data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}

		ensureCapacity(err, allocator, length + string.length - 1);
		TRY(*err);

		copyStringIntoEnd(string);
	}

	void free(Allocator allocator) {
		if (data != NULL) {
			allocator.freeSlice(allocator.context, data, sizeof(char), capacity);
		}

		data = NULL;
		length = 0;
		capacity = 0;
	}

	void clear() {
		length = 0;
	}

	void removeAt(Error* err, size_t index) {

		if (data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}

		if (index >= length) {
			err->setError(ErrorCode::DynamicBuffer_Index_Out_Of_Bounds);
			return;
		}

		for (size_t i = index; i < length - 1; i++) {
			data[i] = data[i + 1];
		}

		length -= 1;
	}

	void removeAtSwap(Error* err, size_t index) {

		if (data == NULL) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return;
		}

		if (index >= length) {
			err->setError(ErrorCode::DynamicBuffer_Index_Out_Of_Bounds);
			return;
		}

		data[index] = data[length - 1];
		length -= 1;
	}

	String toString(Error* err, Allocator allocator) {
		if (data == NULL || length == 0) {
			err->setError(ErrorCode::DynamicBuffer_Null_Data);
			return String::null();
		}

		Slice<char> new_slice = Slice<char>::create(data, length);
		for (size_t i = 0; i < length; i++) {
			new_slice.data[i] = data[i];
		}

		return String::create(err, allocator, new_slice);
	}
};

template <typename T>
struct Iterator {
	Slice<T> slice;
	size_t current_index;

	Optional<T> next() {
		if (current_index >= slice.length) {
			return nullOptional<T>();
		}
		size_t index = current_index;
		current_index += 1;
		return valueOptional<T>(slice.data[index]);
	}
	Optional<T> previous() {
		if (current_index == 0) {
			return nullOptional<T>();
		}
		size_t index = current_index;
		current_index -= 1;
		return valueOptional<T>(slice.data[index]);
	}
	Optional<T> current() {
		if (current_index >= slice.length) {
			return nullOptional<T>();
		}
		return valueOptional<T>(slice.data[current_index]);
	}
	void setToStartIndex() {
		current_index = 0;
	}
	void setIndex(size_t new_index) {
		current_index = new_index;
	}
	void setToLastIndex() {
		current_index = slice.length - 1;
	}
	static Iterator<T> init(Slice<T> _slice) {
		return {
			.slice = _slice,
			.current_index = 0,
		};
	}

};

template <typename T>
Slice<T> createCopySlice(Error* err, Allocator allocator, Slice<T> source) {

	if (source.length == 0) {
		err->setError(ErrorCode::Slice_Copy_Source_Length_Zero);
		return Slice<T>::null();
	}

	Slice<T> destination = allocSlice<T>(err, allocator, source.length);
	TRY_RETURN(*err, destination);
	destination.copySlice(err, source);
	return destination;
}

template <typename T>
Slice<T> createCopySliceFromArray(Error* err, Allocator allocator, T array[], size_t size) {

	if (size == 0) {
		err->setError(ErrorCode::Slice_Copy_Source_Length_Zero);
		return;
	}
	Slice<T> source = Slice<T>::create(array, size);
	Slice<T> destination = allocSlice<T>(err, allocator, source.length);
	TRY_RETURN(*err, destination);
	destination.copySlice(err, source);
	return destination;
}

enum class FileOpenMode {
	Read,
	Write,
	Append,
	ReadWrite,
	WriteRead,
	AppendRead,
};

struct File {
	FILE* file;

	static File create() {
		return File{
			.file = NULL
		};
	}

	void fileOpen(Error* err, Slice<char> path, FileOpenMode mode) {
		if (path.isNull()) {
			err->setError(ErrorCode::String_Null_Data);
			return;
		}

		const char* c_mode = fileModeToCString(mode);
		if (c_mode == NULL) {
			err->setError(ErrorCode::File_Open_Failed);
			return;
		}

		FILE* temp_file = fopen(path.data, c_mode);

		if (temp_file == NULL) {
			err->setError(ErrorCode::File_Open_Failed);
			return;
		}

		file = temp_file;
	}

	void fileRead(Error* err, Slice<void> buffer) {

		if (file == NULL) {
			err->setError(ErrorCode::File_Is_Null);
			return;
		}

		if (buffer.data == NULL) {
			err->setError(ErrorCode::Slice_Null_Data);
			return;
		}

		size_t bytes_read = fread(buffer.data, 1, buffer.length - 1, file);

		if (bytes_read < buffer.length && ferror(file)) {
			err->setError(ErrorCode::File_Read_Failed);
			return;
		}
	}

	String fileReadAllDebug(Error* err, Allocator allocator) {

		printf("fileReadAll() entered\n");
		printf("file ptr: %p\n", file);

		if (file == NULL) {
			printf("ERROR: file is NULL\n");
			err->setError(ErrorCode::File_Is_Null);
			return String::null();
		}

		// Move to end
		int seek_end = fseek(file, 0, SEEK_END);
		printf("fseek SEEK_END result: %d\n", seek_end);

		if (seek_end != 0) {
			printf("ERROR: fseek SEEK_END failed\n");
			err->setError(ErrorCode::File_Read_Failed);
			return String::null();
		}

		long size = ftell(file);
		printf("ftell size: %ld\n", size);

		if (size < 0) {
			printf("ERROR: ftell returned negative\n");
			err->setError(ErrorCode::File_Read_Failed);
			return String::null();
		}

		int rewind_result = fseek(file, 0, SEEK_SET);
		printf("rewind result: %d\n", rewind_result);

		if (rewind_result != 0) {
			printf("ERROR: rewind failed\n");
			err->setError(ErrorCode::File_Read_Failed);
			return String::null();
		}

		size_t alloc_size = (size_t)size + 1;
		printf("allocating %zu bytes\n", alloc_size);

		Slice<char> buffer = allocSlice<char>(err, allocator, alloc_size);

		if (err->isError()) {
			printf("ERROR: allocation failed\n");
			return String::null();
		}

		printf("buffer ptr: %p\n", buffer.data);
		printf("buffer length: %zu\n", buffer.length);

		size_t bytes_read = fread(buffer.data, 1, (size_t)size, file);

		printf("bytes_read: %zu\n", bytes_read);
		printf("expected size: %zu\n", (size_t)size);
		printf("ferror: %d\n", ferror(file));
		printf("feof: %d\n", feof(file));

		if (bytes_read != (size_t)size) {
			printf("ERROR: fread mismatch\n");
			freeSlice(allocator, &buffer);
			err->setError(ErrorCode::File_Read_Failed);
			return String::null();
		}

		buffer.data[size] = '\0';
		printf("null terminator written at index %ld\n", size);

		String result = {
			.data = buffer.data,
			.length = (size_t)size + 1,
		};

		printf("result.data: %p\n", result.data);
		printf("result.length: %zu\n", result.length);
		printf("fileReadAll() success\n");

		return result;
	}

	String fileReadAll(Error* err, Allocator allocator) {

		String null = String::null();

		if (file == NULL) {
			err->setError(ErrorCode::File_Is_Null);
			return null;
		}

		// Move to end
		if (fseek(file, 0, SEEK_END) != 0) {
			err->setError(ErrorCode::File_Read_Failed);
			return null;
		}

		long size = ftell(file);
		if (size < 0) {
			err->setError(ErrorCode::File_Read_Failed);
			return null;
		}

		rewind(file);

		// Allocate +1 for null terminator
		Slice<char> buffer = allocSlice<char>(err, allocator, (size_t)size + 1);
		TRY_RETURN(*err, null);
		
		size_t bytes_read = fread(buffer.data, 1, (size_t)size, file);

		if (bytes_read != (size_t)size) {
			freeSlice(allocator, &buffer);
			err->setError(ErrorCode::File_Read_Failed);
			return null;
		}

		buffer.data[size] = '\0';

		return {
			.data = buffer.data,
			.length = (size_t)size,
		};
	}

	void fileReadIntoBuilder(Error* err, Allocator allocator, StringBuilder* builder) {

		const size_t chunk_size = 4096;

		Slice<char> chunk = allocSlice<char>(err, allocator, chunk_size);
		TRY(*err);
		DEFER(freeSlice(allocator, &chunk););

		while (true) {
			const size_t read_bytes = fread(chunk.data, 1, chunk_size, file);

			if (read_bytes > 0) {
				builder->appendSlice(err, allocator, Slice<char>::create(chunk.data, read_bytes));
				TRY(*err);
			}

			if (read_bytes < chunk_size) {
				if (feof(file)) {
					break;
				}
				err->setError(ErrorCode::File_Read_Failed);
				return;
			}
		}
	}

	void fileWrite(Error* err, const Slice<char> buffer) {

		if (file == NULL) {
			err->setError(ErrorCode::File_Is_Null);
			return;
		}

		if (buffer.data == NULL) {
			err->setError(ErrorCode::Slice_Null_Data);
			return;
		}

		size_t bytes_written = fwrite((const void*)buffer.data, 1, buffer.length, file);

		if (bytes_written != buffer.length) {
			err->setError(ErrorCode::File_Write_Failed);
			return;
		}
	}

	void fileClose(Error* err) {
		if (file == NULL) {
			return;
		}

		if (fclose(file) != 0) {
			err->setError(ErrorCode::File_Close_Failed);
			return;
		}

		file = NULL;
	}

	static const char* fileModeToCString(FileOpenMode mode) {
		switch (mode) {
			case FileOpenMode::Read:
				return "rb";
			case FileOpenMode::Write:
				return "wb";
			case FileOpenMode::Append:
				return "ab";
			case FileOpenMode::ReadWrite:
				return "rb+";
			case FileOpenMode::WriteRead:
				return "wb+";
			case FileOpenMode::AppendRead:
				return "ab+";
			default:
				return NULL;
		}
	}

};

#pragma region Hash

template<typename T>
struct StringHashEntry {
	String key;
	T value;
};

template<typename T>
struct StringHashMap {

	DynamicBuffer<StringHashEntry<T>> entries;
	size_t count;

	Optional<size_t> getEntryIndex(const String* key) {

		for (size_t i = 0; i < entries.length; i++) {
			String entry = entries.data[i].key;
			if (entry.length != key->length) {
				continue;
			}

			if (entry.compare(key) == true) {
				return valueOptional<size_t>(i);
			}
		}
		return nullOptional<size_t>();
	}

	bool hashMapRemove(Error* err, const String* key) {

		Optional<size_t> index = getEntryIndex(key);
		if (index.isNull()) {
			return false;
		}

		entries.removeAt(err, index.value);
		TRY_RETURN(*err, false);
		return true;
	}

	bool append(Error* err, Allocator allocator, const String* key, T value) {
		Optional<size_t> index = getEntryIndex(key);
		if (index.isNull()) {
			return false;
		}

		StringHashEntry<T> entry = {
			.key = *key,
			.value = value,
		};

		entries.append(err, allocator, entry);
		TRY_RETURN(*err, false);

		return true;
	}
	void clear() {
		entries.clear();
	}
};

template<typename T>
StringHashMap<T> createStringHashmap(Error* err, Allocator allocator) {

	StringHashMap<T> null = {
		.data = NULL,
	};

	DynamicBuffer<T> buf;

	Slice<T> data_result = allocSlice<T>(err, allocator, 10);
	TRY_RETURN(*err, null);

	buf.data = data_result.value.data;
	if (buf.data == NULL) {
		return null;
	}

	buf.length = 0;
	buf.capacity = 10;

	StringHashMap<T> hashmap;
	hashmap.entries = buf;

	return hashmap;
}

template<typename K, typename T>
struct HashEntry {
	K key;
	T value;
};

template<typename K, typename T>
struct HashMap {

	DynamicBuffer<HashEntry<K, T>> entries;
	size_t count;

	Optional<size_t> getEntryIndex(K key) {

		for (size_t i = 0; i < entries.length; i++) {
			K entry = entries.data[i];
			if (entry == key) {
				return valueOptional<size_t>(i);
			}
		}
		return nullOptional<size_t>();
	}

	bool hashMapRemove(K key) {

		Optional<size_t> index = getEntryIndex(key);
		if (index.isNull()) {
			return false;
		}

		Error err = entries.removeAt(index.value);
		TRY_RETURN(*err, false);
		return true;
	}

	bool append(Error* err, String key, T value) {
		Optional<size_t> index = getEntryIndex(key);
		if (index.isNull()) {
			return false;
		}

		StringHashEntry<T> entry = {
			.key = key,
			.value = value,
		};

		entries.append(err, entry);
		TRY_RETURN(*err, false);
		return true;
	}
	void clear() {
		entries.clear();
	}
};

template<typename K, typename T>
HashMap<K, T> createHashmap(Error* err, Allocator allocator) {

	DynamicBuffer<T> buf;
	HashMap<K, T> null = {
		.entries = DynamicBuffer<T>::null(),
		.count = 0,
	};

	Slice<T> data = allocSlice<T>(err, allocator, 10);
	TRY_RETURN(*err, null);

	buf.data = data.value.data;

	buf.length = 0;
	buf.capacity = 10;

	StringHashMap<T> hashmap;
	hashmap.entries = buf;

	Result<StringHashMap<T>> result;
	result.value = hashmap;
	result.err = Error{ .code = ErrorCode::None };

	return result;
}

enum class HashEntryState {
	Empty,
	Occupied,
	Tombstone
};

template<typename T>
struct Hashset {

	DynamicBuffer<T> buffer;

	static Hashset null() {
		return {
			.buffer = DynamicBuffer<T>::null(),
		};
	}

	bool exists(T value) {

		for (size_t i = 0; i < buffer.length; i++) {
			if (buffer.data[i] == value) {
				return true;
			}
		}
		return false;
	}
	Optional<size_t> getIndexOf(T value) {

		for (size_t i = 0; i < buffer.length; i++) {
			if (buffer.data[i] == value) {
				return valueOptional<size_t>(i);
			}
		}
		return nullOptional<size_t>();
	}
	size_t count() {
		return buffer.length;
	}
	bool append(Error* err, Allocator allocator, T value) {
		if (exists(value)) {
			return false;
		}
		buffer.append(err, allocator, value);
		TRY_RETURN(*err, false);
		return true;
	}
	bool removeEntry(T value) {
		Optional<size_t> index = getIndexOf(value);
		if (index.isNull()) {
			return false;
		}
		Error err = Error::init();
		buffer.removeAt(&err, index.value);
		TRY_RETURN(err, false);
		return true;
	}
	void clear() {
		buffer.clear();
	}
	void clearHashSet(Allocator allocator) {
		return buffer.freeDynBuffer(allocator);
	}
};

template<typename T>
Hashset<T> createHashset(Error* err, Allocator allocator) {

	DynamicBuffer<T> buf;
	Hashset<T> null = Hashset<T>::null();

	Slice<T> data_result = allocSlice<T>(err, allocator, 10);
	TRY_RETURN(*err, null);

	buf.data = data_result.value.data;
	if (buf.data == NULL) {
		err->setError(ErrorCode::Allocation_Failed);
		return null;
	}

	buf.length = 0;
	buf.capacity = 10;

	Hashset<T> hashset;
	hashset.buffer = buf;
	return hashset;
}

#pragma endregion