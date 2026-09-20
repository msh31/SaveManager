#pragma once

using Value = std::variant<bool, std::int64_t, double, std::string>;

enum class FieldType {
	BOOLEAN,
	INTEGER,
	FLOAT,
	STRING
};

enum class WidgetHint {
	TOGGLE,
	SLIDER,
	NUMBER,
	TEXT
};

struct EditorFieldDescriptor {
	std::string id;
	std::string label;
	std::string group;

	FieldType type = FieldType::BOOLEAN;
	WidgetHint widget = WidgetHint::TOGGLE;

	double min = 0.0;
	double max = 3.0;
	
	bool read_only = false;
};

class ISaveEditor {
    public:
        virtual ~ISaveEditor( ) = default;

        virtual bool open( fs::path path ) = 0;
        virtual bool save( fs::path path ) = 0;
        virtual void close( ) = 0;

		virtual std::optional<Value> get( const std::string& id ) const = 0;
        virtual bool set( const std::string& id, const Value& val ) = 0;

		virtual const std::vector<EditorFieldDescriptor>& fields( ) const = 0;
};