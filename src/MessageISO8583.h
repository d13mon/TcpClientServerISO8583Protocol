#ifndef MESSAGEISO8583_H
#define MESSAGEISO8583_H

#include "ByteConversionHelper.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <bitset>
#include <cassert>
#include <variant>
#include <iostream>


namespace Iso8583 {	

	constexpr short BITMAP_SIZE = 64;
	using Bitmap = std::bitset<BITMAP_SIZE>;

	using DataSize = uint16_t;

	using ull = unsigned long long;

	using FieldType = std::variant<ull, std::string, vbytes>;

	inline bool isBitmapFieldSet(const Bitmap& bitmap, int fieldNumber) {
		assert(fieldNumber > 0 && fieldNumber <= static_cast<int>(bitmap.size()));
		return bitmap[bitmap.size() - fieldNumber];
	}

	class parse_from_bytes_error : public std::out_of_range {
	public:
		parse_from_bytes_error(const std::string& what);
	};

	class bad_field_data_type : public std::exception {
	public:
		bad_field_data_type();
	};


	class IData {
	public:			
		virtual DataSize size() const = 0;
		virtual bool isNull() const = 0;		
		virtual vbytes toBytes() const = 0;
		virtual void print(std::ostream& os) const = 0;
	};	


	class IField: public IData {		
	public:
		explicit IField(short number);
		virtual ~IField();
	public:	
		inline bool isNull() const override { return mNumber == 0; }	
		vbytes toBytes() const override;
		
		inline short number() const { return mNumber; }
		static bool checkNumber(short number);

		void print(std::ostream& os) const override;

		inline static constexpr size_t maxFieldNumber() { return 128; }
		inline static constexpr size_t maxFieldSize() { return 1000; }
	public:	
		
        virtual vbytes getDataBytes() const = 0;
		virtual void setData(const FieldType& data) = 0;
		virtual void setData(const vbytes& data) = 0;	
		virtual std::string toString() = 0;
		
	protected:
		short mNumber = 0;		
	};
	
	using FieldPtr = std::shared_ptr<IField>;
	using FieldWPtr = std::weak_ptr<IField>;
	using FieldSet = std::set<FieldPtr>;

	inline bool operator < (const FieldPtr& left, const FieldPtr& right) {
		return left->number() < right->number();
	}		

	class NumberField : public IField {
	public:
		explicit NumberField(short number);
		NumberField(short number, ull data);			
		
		inline ull data() { return sizeof(mData); }
		inline void setData(const FieldType& data) override { mData = std::get<ull>(data); }
		inline void setData(const vbytes& data) override { from_bytes(data, mData); }
		inline DataSize size() const override { return static_cast<DataSize>(sizeof(mData)); }
		inline vbytes getDataBytes() const override { return to_bytes(mData); }
		inline std::string toString() override { return std::to_string(mData); }
		void print(std::ostream& os) const override;

	protected:
		ull mData;
	};

	
	class StringField : public IField {
	public:
		explicit StringField(short number);
		StringField(short number, const std::string& data);

		inline std::string data() { return mData; }	
		inline void setData(const FieldType& data) override { mData = std::get<std::string>(data); }
		inline void setData(const vbytes& data) override { from_bytes(data, mData); }
		inline DataSize size() const override { return static_cast<DataSize>(mData.size()); }	
		inline vbytes getDataBytes() const override { return to_bytes(mData); }	
		inline std::string toString() override { return  mData; }
		void print(std::ostream& os) const override;

	protected:
		std::string mData;
	};	


	class ByteField : public IField {
	public:
		explicit ByteField(short number);		
		ByteField(short number, const vbytes& data);
		ByteField(short number, const std::string& data);		

		inline vbytes data() { return mData; }
		inline void setData(const FieldType& data) override { mData = std::get<vbytes>(data); }
		inline void setData(const vbytes& data) override { mData = data; }
		inline DataSize size() const override { return static_cast<DataSize>(mData.size()); }
		inline vbytes getDataBytes() const override { return mData; }
		std::string toString() override;
		void print(std::ostream& os) const override;

	protected:
		vbytes mData;
	};		


	class MTIField : public StringField {
	public:
		explicit MTIField(const std::string& mti);		
		
		vbytes toBytes() const override;	
		void print(std::ostream& os) const override;
	};


	class Message: public IData {
	public:			
		explicit Message(const std::string& mti = {});
		virtual ~Message();
		
		std::string getMTI();
		void setMTI(const std::string mti);
		
		static Message fromBytes(const vbytes& bytes);	

	public:		
		bool setField(short number, const FieldType& data);
		bool setField(const FieldPtr& field);	

		FieldPtr field(short number) const;		
		
		static FieldPtr createField(short fieldNumber);		
		static FieldPtr createFieldFromBytes(short fieldNumber, const vbytes& dataBytes);
		
		FieldSet getFieldSet() const;

	public://IData
		bool isNull() const override;
		DataSize size() const override;		
		vbytes toBytes() const override;
		void print(std::ostream& os) const override;

	public:			
		static inline constexpr size_t bitmapSizeBytes() { return BITMAP_SIZE / 8; }
		static inline constexpr size_t defaultMTISizeBytes() { return 4; }			

	private:
		bool isSecondBitmapPresent() const;
		void setSecondBitmapPresent();		
		
		static std::string readMti(const vbytes& bytes, size_t& readPos);
		static std::pair<Bitmap, Bitmap> readBitmaps(const vbytes& bytes, size_t& readPos);		
		static FieldSet readFields(const vbytes& bytes, size_t& readPos, const std::vector<short>& fieldNumbers);
		static void readAndCopyBytes(const vbytes& fromBytes, size_t& readPos, size_t readCount, vbytes& toBytes);		

	private:
		MTIField mMTI;
		Bitmap mBitmap1, mBitmap2;
		std::unordered_map<short, FieldPtr> mFields;		
	};


	inline std::ostream& operator<<(std::ostream& os, const Message& message) {
		message.print(os);
		return os;
	}
}

#endif



