
var surface = new SurfaceInterface()
surface.onUpdate = function(node) {
    print("SurfaceInterface::onUpdate called", node);
    return undefined;
}

surface.show();

