SystemOverview::SystemOverview(View* parent) : Panel(parent, Size{~0, ~0}) {
  setCollapsible().setVisible(false);
  spacerButton.setVisible(false);
}

auto SystemOverview::show() -> void {
  setVisible(true);
}

auto SystemOverview::hide() -> void {
  setVisible(false);
}

auto SystemOverview::refresh() -> void {
  auto location = systemManager.systemList.selected().property("location");
  nodeList.reset();
  auto identity = BML::unserialize(file::read({location, "identity.bml"}));
  auto root = higan::Node::unserialize(file::read({location, "node.bml"}));
  nodeList.append(ListViewItem().setText(identity["system"].text()));
  for(auto& node : *root) scan(node);
}

auto SystemOverview::scan(higan::Node::Object node, uint depth) -> void {
  if(node->is<higan::Node::Input>()) return;
  if(node->is<higan::Node::Component>() && !settingsMenu.showComponents.checked()) return;
  if(node->is<higan::Node::Settings>() && node->name == "Hacks" && !settingsMenu.showHacks.checked()) return;

  ListViewItem item{&nodeList};
  string name;
  for(uint n : range(depth)) name.append("   ");
  name.append(node->property("name") ? node->property("name") : node->name);
  if(auto setting = node->cast<higan::Node::Setting>()) {
    name.append(": ", setting->getValue());
  }

  item.setText(name);
  for(auto& node : *node) scan(node, depth + 1);
}
