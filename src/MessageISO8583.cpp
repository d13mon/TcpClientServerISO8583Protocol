#include "MessageISO8583.h"

#include <iostream>
#include <exception>
#include <cstring>
#include <sstream>
#include <iomanip>

using namespace std;

namespace Iso8583 {


	parse_from_bytes_error::parse_from_bytes_error(const std::string& what)
		: std::out_of_range(what)
	{}

	bad_field_data_type::bad_field_data_type()
		: std::exception("Incorrect field data type.")
	{}


	//==============================class IField============================================================

	IField::IField(short number)
		: mNumber(number)		
	{
		if (!checkNumber(number))
			throw std::out_of_range("Field number is out of range");
	}

	IField::~IField()
	{
	}		

	bool IField::checkNumber(short number)
	{
		return number >= 0 && number <= static_cast<short>(maxFieldNumber());
	}	

	void IField::print(std::ostream& os) const
	{
		os << "Field #" << number() << ": "; 
	}

	vbytes IField::toBytes() const
	{
		vbytes bytes;	
		auto sizeBytes = to_bytes(size());
		std::copy(begin(sizeBytes), end(sizeBytes), back_inserter(bytes));	

		auto dataBytes = getDataBytes();
		std::copy(begin(dataBytes), end(dataBytes), back_inserter(bytes));

		return bytes;
	}

	//==============================class NumberField ============================================================

	NumberField::NumberField(short number)
		: IField(number) {}	

	NumberField::NumberField(short number, ull data)
		: IField(number)
		, mData(data) {}	

	void NumberField::print(std::ostream& os) const
	{
		IField::print(os);
		os << mData << endl;
	}


	//==============================class StringField ============================================================
	
	StringField::StringField(short number)
		: IField(number)
	{}

	StringField::StringField(short number, const std::string& data)
		: IField(number)
		, mData(data)
	{}

	void StringField::print(std::ostream& os) const
	{
		IField::print(os);
		os << mData << endl;
	}

	//==============================class ByteField ============================================================

	ByteField::ByteField(short number)
		: IField(number)
	{}

	ByteField::ByteField(short number, const vbytes& data)
		: IField(number)
		, mData(data)
	{}

	ByteField::ByteField(short number, const std::string& data)
		: IField(number)
	{
		mData = to_bytes(data);
	}	

	std::string ByteField::toString()
	{
		std::stringstream ss;		

		for (size_t i{ 0 }; i < mData.size(); ++i) {
			ss << static_cast<int>(mData[i]);           
		}				

		return ss.str();
	}

	void ByteField::print(std::ostream& os) const
	{
		IField::print(os);
		
		os << std::hex << std::setfill('0');
		for (auto b : mData) std::cout << std::setw(2) << int(b) << ' ';
		cout << std::dec << endl;
	}

	//==============================class MTIField ============================================================

	MTIField::MTIField(const std::string& mti)
		: StringField(0, mti)
	{
	}

	vbytes MTIField::toBytes() const
	{
		vbytes bytes;
		auto dataBytes = getDataBytes();		
		std::copy(begin(dataBytes), end(dataBytes), back_inserter(bytes));

		return bytes;
	}	

	void MTIField::print(std::ostream& os) const
	{
		os << "MTI: " << mData << endl;
	}

	//==============================class Message============================================================

	Message::Message(const std::string& mti)
		: mMTI(MTIField(mti))
	{
	}

	Message::~Message()
	{
	}

	bool Message::setField(const FieldPtr& field)
	{
		if (field && !field->isNull()) {
			auto number = field->number();
			mFields[number] = field;

			if (number <= BITMAP_SIZE) {
				mBitmap1.set(mBitmap1.size() - number);
			}
			else
			{
				setSecondBitmapPresent();
				auto bitInBitmap2 = number - BITMAP_SIZE;
				mBitmap2.set(mBitmap2.size() - bitInBitmap2);
			}
#if 0
			cout << mBitmap1 << " " << mBitmap2 << endl;
#endif
			return true;
		}

		return false;
	}

	bool Message::setField(short number, const FieldType& data)
	{
		try {
			if (auto field = createField(number)) {
				field->setData(data);
				return setField(field);
			}			
		}
		catch (const std::bad_variant_access) {
			throw bad_field_data_type();
		}

		return false;
	}

	FieldPtr Message::field(short number) const
	{
		if (mFields.count(number)) {
			return mFields.at(number);
		}

		return FieldPtr{};
	}	

	FieldPtr Message::createField(short fieldNumber)
	{
		if (!IField::checkNumber(fieldNumber))
			return FieldPtr{};

		switch (fieldNumber) {	
//NOTE: You can use also numeric fields as follows. By default almost all fields are StringField
#if 0
		case 3: return FieldPtr(new NumberField(fieldNumber));
#endif
		case 52:
		case 64:
		case 65:
		case 128: return FieldPtr(new ByteField(fieldNumber));

		default:
			break;
		}

		return FieldPtr(new StringField(fieldNumber));
	}

	FieldSet Message::getFieldSet() const
	{
		FieldSet fieldSet;
		std::transform(begin(mFields), end(mFields), inserter(fieldSet, begin(fieldSet)), [](const auto& node) {
			return node.second;
		});

		return std::move(fieldSet);
	}

	std::string Message::getMTI()
	{
		return mMTI.data();
	}

	void Message::setMTI(const std::string mti)
	{
		mMTI.setData(mti);
	}

	bool Message::isNull() const
	{
		return mFields.size() == 0;
	}

	DataSize Message::size() const
	{
		size_t size = mMTI.size();

		size += bitmapSizeBytes();

		if(isSecondBitmapPresent()) 
			size += bitmapSizeBytes();

		for (const auto & field : mFields) {
			size += field.second->size();
		}

		return static_cast<DataSize>(size);
	}

	vbytes Message::toBytes() const
	{
		vbytes bytes;		

		auto bytesMti = mMTI.toBytes();
		std::copy(begin(bytesMti), end(bytesMti), back_inserter(bytes));		

		auto bytesBitmap1 = to_bytes(mBitmap1);
		std::copy(begin(bytesBitmap1), end(bytesBitmap1), back_inserter(bytes));

		if (isSecondBitmapPresent()) {
			auto bytesBitmap2 = to_bytes(mBitmap2);
			std::copy(begin(bytesBitmap2), end(bytesBitmap2), back_inserter(bytes));
		}		

		auto fieldSet = getFieldSet();
		for (const auto & field : fieldSet) {
			auto fieldBytes = field->toBytes();
			std::copy(begin(fieldBytes), end(fieldBytes), back_inserter(bytes));
		}	
#if 0
		cout << std::dec << "Message::toBytes: Total bytes = " << bytes.size() << endl;	
#endif	
		return std::move(bytes);
	}


	void Message::print(std::ostream& os) const
	{
		mMTI.print(os);
		auto fields = getFieldSet();
		for (const auto& field : fields) {
			field->print(os);
		}
	}

	bool Message::isSecondBitmapPresent() const
	{
		return isBitmapFieldSet(mBitmap1,1);
	}

	void Message::setSecondBitmapPresent()
	{
		mBitmap1.set(mBitmap1.size() - 1);
	}

	Message Message::fromBytes(const vbytes& bytes)
	{
		auto size = bytes.size();
		if (size == 0) return Message{};		

		size_t readPos = 0;

		if (size < defaultMTISizeBytes())
			throw std::out_of_range("Byte buffer format is incorrect (MTI)");

		auto MTI = readMti(bytes, readPos);
		
		Message message{ MTI };		

		auto bitmaps = readBitmaps(bytes, readPos);
		bool hasSecondBitmap = isBitmapFieldSet(bitmaps.first, 1);

		std::vector<short> fieldNumbers;
		for (int i = BITMAP_SIZE-2; i > 0; i--) {
			auto fieldNumber = BITMAP_SIZE - i;			

			if (bitmaps.first[i]) {
				fieldNumbers.push_back(fieldNumber);
			}
		}

		if (hasSecondBitmap) {
			for (int i = BITMAP_SIZE - 1; i > 0; i--) {
				auto fieldNumber = 2*BITMAP_SIZE - i;

				if (bitmaps.second[i]) {
					fieldNumbers.push_back(fieldNumber);
				}
			}
		}

		auto fields = readFields(bytes, readPos, fieldNumbers);

		for (const auto & field : fields) {
			message.setField(field);
		}

		return std::move(message);
	}
	
	void Message::readAndCopyBytes(const vbytes& fromBytes, size_t& readPos, size_t readCount, vbytes& toBytes)
	{
		std::copy(begin(fromBytes) + readPos, begin(fromBytes) + readPos + readCount, back_inserter(toBytes));
		readPos += readCount;
	}

	string Message::readMti(const vbytes& bytes, size_t& readPos)
	{
		char fieldBuffer[IField::maxFieldSize()];

		vbytes mtiBytes;
		readAndCopyBytes(bytes, readPos, defaultMTISizeBytes(), mtiBytes);		

		from_bytes(mtiBytes, fieldBuffer);
		std::string mti(fieldBuffer, defaultMTISizeBytes());	
		
		return std::move(mti);
	}

	std::pair<Bitmap, Bitmap> Message::readBitmaps(const vbytes& bytes, size_t& readPos)
	{
		vbytes bitmapBytes;

		if (bytes.size() < readPos + bitmapSizeBytes())
			throw parse_from_bytes_error("Byte buffer format is incorrect (Bitmap1)");	

		readAndCopyBytes(bytes, readPos, bitmapSizeBytes(), bitmapBytes);

		unsigned long long bitmap1Value = 0;
		unsigned long long bitmap2Value = 0;
		from_bytes(bitmapBytes, bitmap1Value);
		Bitmap bitmap1(bitmap1Value);
		Bitmap bitmap2;
#if 0
		cout << bitmap1 << endl;
#endif
		if (isBitmapFieldSet(bitmap1, 1)) {
			bitmapBytes.clear();

			if (bytes.size() < readPos + bitmapSizeBytes())
				throw std::out_of_range("Byte buffer format is incorrect (Bitmap2)");

			readAndCopyBytes(bytes, readPos, bitmapSizeBytes(), bitmapBytes);

			from_bytes(bitmapBytes, bitmap2Value);
			bitmap2 = Bitmap(bitmap2Value);

#if 0
			cout << bitmap2 << endl;
#endif
		}	

		return  { bitmap1, bitmap2 };
	}	

	FieldSet Message::readFields(const vbytes& bytes, size_t& readPos, const std::vector<short>& fieldNumbers)
	{
		FieldSet fields;

		for (auto number : fieldNumbers) {
			vbytes sizeBytes, typeBytes, dataBytes;
			DataSize fieldSize;

			if (bytes.size() < readPos + 2*sizeof(DataSize))
				throw parse_from_bytes_error("Byte buffer format is incorrect (Fields)");						

			readAndCopyBytes(bytes, readPos, sizeof(DataSize), sizeBytes);
			from_bytes(sizeBytes, fieldSize);

			if (fieldSize < 0 || fieldSize > IField::maxFieldSize())
				throw parse_from_bytes_error("Field size error");		

			readAndCopyBytes(bytes, readPos, fieldSize, dataBytes);

			auto field = createFieldFromBytes(number, dataBytes);
			assert(field);
			if(!field)
				throw parse_from_bytes_error("Can't create field");

			fields.insert(std::move(field));			
		}	

		return std::move(fields);
	}

	FieldPtr Message::createFieldFromBytes(short fieldNumber, const vbytes& dataBytes)
	{
#if 0
		cout << "CreateField: Number=" << fieldNumber << " sizeof(data) = " << dataBytes.size() << endl;
#endif		
		if (auto field = createField(fieldNumber)) {
			field->setData(dataBytes);
			return field;
		}

		return FieldPtr{};
	}	



}//namespace

