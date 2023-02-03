#include "Gui.h"
#include "Graphic.h"
#include "Default_Programs.h"

Layout::Layout(uint32_t type, const AABB2& aabb) :
	type((Layout::LayoutType)type), window_size(aabb.size), position(aabb.position) {}

Layout::Layout(uint32_t type, const Vec2<float>& position, const Vec2<float>& window_size) :
	type((Layout::LayoutType)type), window_size(window_size), position(position) {}

void Layout::add_widget(const Vec2<float>& size){
	if (type == Layout::Horizional) {
		window_size.x += size.x;
		window_size.y = std::max(window_size.y, size.y);
	}
	else if (type == Layout::Vertical) {
		window_size.x = std::max(window_size.x, size.x);
		window_size.y += size.y;
	}
}

Vec2<float> Layout::get_widget_position() {
	if (type == Layout::Horizional) {
		return Vec2<float>(position.x + window_size.x, position.y);
	}
	else if (type == Layout::Vertical) {
		return Vec2<float>(position.x, position.y + window_size.y);
	}
}

//StaticStyle& Style::get_hover(){
//	return _on_hover;
//}
//
//StaticStyle& Style::get_active() {
//	return _on_active;
//}

Ui::Ui(Frame& frame) :
	frame(frame)
{
	window_size = Vec2<int>(frame.window_width, frame.window_height);
	_update_matrix(frame.window_width, frame.window_height);
	program = default_program::gui_program();
}

void Ui::new_frame(){
	
	if (window_size.x != frame.window_width || window_size.y != frame.window_height) {
		window_size = Vec2<int>(frame.window_width, frame.window_height);
		_update_matrix(frame.window_width, frame.window_height);
	}

	layouts.clear();

	Frame::CursorState cursor_state = frame.get_mouse_state();
	if (!_hovered)
		frame.set_cursor_type(Frame::Arrow);
	_hovered = false;

	if (_cursor_state == cursor_state)
		_cursor_state_just_changed = false;

	else if (_cursor_state != cursor_state)
		_cursor_state_just_changed = true;
	
	_cursor_state = cursor_state;

}

void Ui::begin(Layout::LayoutType type){
	Vec2<float> position;
	if (layouts.size() > 0)
		position = layouts.back().get_widget_position();
	layouts.push_back(Layout(type, position));
}

bool Ui::box(const std::string& id, const Vec2<float>& size, const Style& style){
	
	// determine aabb
	Vec2<float> screen_position = layouts.back().get_widget_position();
	layouts.back().add_widget(size);
	AABB2 box_aabb(screen_position, size);
	
	bool hover = box_aabb.does_contain(frame.get_cursor_position());
	// hover/active/passive update
	if (hover) {
		if ((_cursor_state == Frame::LeftPressed || _cursor_state == Frame::RightPressed) && _cursor_state_just_changed) {
			_focused_id = id;
		}
	}

	bool active = _focused_id == id;
	StaticStyle style_to_use = (StaticStyle)style;
	if (active) {
		style_to_use = (StaticStyle)style.on_active;
		if (hover)
			frame.set_cursor_type(style_to_use.cursor_type.value_or(style.cursor_type.value_or(Frame::Arrow)));
	}
	else if (hover){
		_hovered = true;
		style_to_use = (StaticStyle)style.on_hover;
		frame.set_cursor_type(style_to_use.cursor_type.value_or(style.cursor_type.value_or(Frame::Arrow)));
	}
	

	// render
	Mesh box_mesh(box_aabb.generate_model());
	Mesh_s box_mesh_s = Mesh_s(box_mesh);
	Program_s program_s = Program_s(program);
	Graphic box_graphic;

	box_graphic.load_program(program_s);
	box_graphic.load_model(box_mesh_s);
	box_graphic.set_uniform("screen_position", screen_position.x, frame.window_height - screen_position.y);
	box_graphic.set_uniform("projection", projection_matrix);
	box_graphic.set_uniform("color", 
		style_to_use.color.value_or(style.color.value_or(Vec3<float>(0, 0, 0))).x,
		style_to_use.color.value_or(style.color.value_or(Vec3<float>(0, 0, 0))).y,
		style_to_use.color.value_or(style.color.value_or(Vec3<float>(0, 0, 0))).z, 1.0f);
	box_graphic.set_uniform("rect_size", size.x, size.y);
	box_graphic.set_uniform("corner_rounding", 
		style_to_use.corner_rounding.value_or(style.corner_rounding.value_or(Vec4<float>(0, 0, 0, 0))).x,
		style_to_use.corner_rounding.value_or(style.corner_rounding.value_or(Vec4<float>(0, 0, 0, 0))).y,
		style_to_use.corner_rounding.value_or(style.corner_rounding.value_or(Vec4<float>(0, 0, 0, 0))).z,
		style_to_use.corner_rounding.value_or(style.corner_rounding.value_or(Vec4<float>(0, 0, 0, 0))).w);
	box_graphic.set_uniform("border_color",
		style_to_use.border_color.value_or(style.border_color.value_or(Vec3<float>(0, 0, 0))).x,
		style_to_use.border_color.value_or(style.border_color.value_or(Vec3<float>(0, 0, 0))).y,
		style_to_use.border_color.value_or(style.border_color.value_or(Vec3<float>(0, 0, 0))).z,
		1.0f);
	box_graphic.set_uniform("border_thickness",
		style_to_use.border_thickness.value_or(style.border_thickness.value_or(Vec4<float>(0, 0, 0, 0))).x,
		style_to_use.border_thickness.value_or(style.border_thickness.value_or(Vec4<float>(0, 0, 0, 0))).y,
		style_to_use.border_thickness.value_or(style.border_thickness.value_or(Vec4<float>(0, 0, 0, 0))).z,
		style_to_use.border_thickness.value_or(style.border_thickness.value_or(Vec4<float>(0, 0, 0, 0))).w);
	

	box_graphic.update_matrix();
	box_graphic.update_uniforms();
	box_graphic.draw(false);
}

void Ui::end() {
	Vec2<float> size = layouts.back().window_size;
	layouts.pop_back();
	layouts.back().add_widget(size);
}

void Ui::_update_matrix(int screen_width, int screen_height) {
	projection_matrix = glm::ortho(0.0f, (float)screen_width, 0.0f, (float)screen_height);
}