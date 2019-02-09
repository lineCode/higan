auto Mouse::create() -> Node::Peripheral {
  auto node = Node::Peripheral::create("Mouse");
  node->append<Node::Axis>("X-Axis");
  node->append<Node::Axis>("Y-Axis");
  node->append<Node::Button>("Left");
  node->append<Node::Button>("Right");
  return node;
}

Mouse::Mouse(Node::Peripheral peripheral) {
  node  = peripheral;
  x     = node->find<Node::Axis>("X-Axis");
  y     = node->find<Node::Axis>("Y-Axis");
  left  = node->find<Node::Button>("Left");
  right = node->find<Node::Button>("Right");

  latched = 0;
  counter = 0;

  speed = 0;
  cx = 0;
  cy = 0;
  dx = 0;
  dy = 0;
}

auto Mouse::data() -> uint2 {
  if(latched == 1) {
    speed = (speed + 1) % 3;
    return 0;
  }

  if(counter >= 32) return 1;

  switch(counter++) { default:
  case  0: return 0;
  case  1: return 0;
  case  2: return 0;
  case  3: return 0;
  case  4: return 0;
  case  5: return 0;
  case  6: return 0;
  case  7: return 0;

  case  8: return right->value;
  case  9: return left->value;
  case 10: return speed.bit(1);
  case 11: return speed.bit(0);

  case 12: return 0;  //4-bit device signature
  case 13: return 0;
  case 14: return 0;
  case 15: return 1;

  case 16: return dy;
  case 17: return cy.bit(6);
  case 18: return cy.bit(5);
  case 19: return cy.bit(4);
  case 20: return cy.bit(3);
  case 21: return cy.bit(2);
  case 22: return cy.bit(1);
  case 23: return cy.bit(0);

  case 24: return dx;
  case 25: return cx.bit(6);
  case 26: return cx.bit(5);
  case 27: return cx.bit(4);
  case 28: return cx.bit(3);
  case 29: return cx.bit(2);
  case 30: return cx.bit(1);
  case 31: return cx.bit(0);
  }
}

auto Mouse::latch(bool data) -> void {
  if(latched == data) return;
  latched = data;
  counter = 0;

  platform->inputPoll(x);  //-n = left, 0 = center, +n = right
  platform->inputPoll(y);  //-n = up,   0 = center, +n = down
  platform->inputPoll(left);
  platform->inputPoll(right);

  cx = x->value;
  cy = y->value;

  dx = cx < 0;  //0 = right, 1 = left
  dy = cy < 0;  //0 = down,  1 = up

  if(cx < 0) cx = -cx;  //abs(position_x)
  if(cy < 0) cy = -cy;  //abs(position_y)

  double multiplier = 1.0;
  if(speed == 1) multiplier = 1.5;
  if(speed == 2) multiplier = 2.0;
  cx = (double)cx * multiplier;
  cy = (double)cy * multiplier;

  cx = min(127, cx);
  cy = min(127, cy);
}
