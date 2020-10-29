//
//  SDL_uikitviewcontroller+Additions.swift
//  disasteroids3d-iOS
//
//  Created by Tom Kidd on 8/26/20.
//  Copyright © 2020 Tom Kidd. All rights reserved.
//

import Foundation

extension SDL_uikitviewcontroller {
    
    // A method of getting around the fact that Swift extensions cannot have stored properties
    // https://medium.com/@valv0/computed-properties-and-extensions-a-pure-swift-approach-64733768112c
    struct Holder {
        static var _fireButton = UIButton()
        static var _thrustButton = UIButton()
        static var _rotateLeftButton = UIButton()
        static var _rotateRightButton = UIButton()
        static var _shieldsButton = UIButton()
        static var _joystickView = JoyStickView(frame: .zero)
        static var _enterButton = UIButton()
    }
    
    var fireButton:UIButton {
        get {
            return Holder._fireButton
        }
        set(newValue) {
            Holder._fireButton = newValue
        }
    }
    
    var thrustButton:UIButton {
        get {
            return Holder._thrustButton
        }
        set(newValue) {
            Holder._thrustButton = newValue
        }
    }
    
    var rotateLeftButton:UIButton {
        get {
            return Holder._rotateLeftButton
        }
        set(newValue) {
            Holder._rotateLeftButton = newValue
        }
    }
    
    var rotateRightButton:UIButton {
        get {
            return Holder._rotateRightButton
        }
        set(newValue) {
            Holder._rotateRightButton = newValue
        }
    }
    
    var shieldsButton:UIButton {
        get {
            return Holder._shieldsButton
        }
        set(newValue) {
            Holder._shieldsButton = newValue
        }
    }
    
    var joystickView:JoyStickView {
        get {
            return Holder._joystickView
        }
        set(newValue) {
            Holder._joystickView = newValue
        }
    }
    
    var enterButton:UIButton {
        get {
            return Holder._enterButton
        }
        set(newValue) {
            Holder._enterButton = newValue
        }
    }

    @objc func fireButton(rect: CGRect) -> UIButton {
        fireButton = UIButton(frame: CGRect(x: rect.width - 70, y: rect.height - 90, width: 70, height: 70))
        fireButton.setTitle("FIRE", for: .normal)
        fireButton.titleLabel?.font = UIFont(name: "DINCondensed-Bold", size: 20)
        fireButton.titleLabel?.numberOfLines = 0
        fireButton.titleLabel?.textAlignment = .center
        fireButton.setTitleColor(.black, for: .normal)
        fireButton.titleEdgeInsets = UIEdgeInsets(top: 5, left: 0, bottom: 0, right: 0)
        fireButton.setBackgroundImage(UIImage(named: "JoyStickBase")!, for: .normal)
        fireButton.addTarget(self, action: #selector(self.firePressed), for: .touchDown)
        fireButton.addTarget(self, action: #selector(self.fireReleased), for: .touchUpInside)
        fireButton.alpha = 0.5
        return fireButton
    }
    
    @objc func thrustButton(rect: CGRect) -> UIButton {
        thrustButton = UIButton(frame: CGRect(x: rect.width - 170, y: rect.height - 90, width: 70, height: 70))
        thrustButton.setTitle("THRUST", for: .normal)
        thrustButton.titleLabel?.font = UIFont(name: "DINCondensed-Bold", size: 20)
        thrustButton.titleLabel?.numberOfLines = 0
        thrustButton.titleLabel?.textAlignment = .center
        thrustButton.setTitleColor(.black, for: .normal)
        thrustButton.titleEdgeInsets = UIEdgeInsets(top: 5, left: 0, bottom: 0, right: 0)
        thrustButton.setBackgroundImage(UIImage(named: "JoyStickBase")!, for: .normal)
        thrustButton.addTarget(self, action: #selector(self.thrustPressed), for: .touchDown)
        thrustButton.addTarget(self, action: #selector(self.thrustReleased), for: .touchUpInside)
        thrustButton.alpha = 0.5
        return thrustButton
    }
    
    @objc func rotateLeftButton(rect: CGRect) -> UIButton {
        rotateLeftButton = UIButton(frame: CGRect(x: 20, y: rect.height - 90, width: 70, height: 70))
        rotateLeftButton.setTitle("ROTATE LEFT", for: .normal)
        rotateLeftButton.titleLabel?.font = UIFont(name: "DINCondensed-Bold", size: 20)
        rotateLeftButton.titleLabel?.numberOfLines = 0
        rotateLeftButton.titleLabel?.textAlignment = .center
        rotateLeftButton.setTitleColor(.black, for: .normal)
        rotateLeftButton.titleEdgeInsets = UIEdgeInsets(top: 10, left: 0, bottom: 0, right: 0)
        rotateLeftButton.setBackgroundImage(UIImage(named: "JoyStickBase")!, for: .normal)
        rotateLeftButton.addTarget(self, action: #selector(self.rotateLeftPressed), for: .touchDown)
        rotateLeftButton.addTarget(self, action: #selector(self.rotateLeftReleased), for: .touchUpInside)
        rotateLeftButton.alpha = 0.5
        return rotateLeftButton
    }
    
    @objc func rotateRightButton(rect: CGRect) -> UIButton {
        rotateRightButton = UIButton(frame: CGRect(x: 120, y: rect.height - 90, width: 70, height: 70))
        rotateRightButton.setTitle("ROTATE RIGHT", for: .normal)
        rotateRightButton.titleLabel?.font = UIFont(name: "DINCondensed-Bold", size: 20)
        rotateRightButton.titleLabel?.numberOfLines = 0
        rotateRightButton.titleLabel?.textAlignment = .center
        rotateRightButton.setTitleColor(.black, for: .normal)
        rotateRightButton.titleEdgeInsets = UIEdgeInsets(top: 10, left: 0, bottom: 0, right: 0)
        rotateRightButton.setBackgroundImage(UIImage(named: "JoyStickBase")!, for: .normal)
        rotateRightButton.addTarget(self, action: #selector(self.rotateRightPressed), for: .touchDown)
        rotateRightButton.addTarget(self, action: #selector(self.rotateRightReleased), for: .touchUpInside)
        rotateRightButton.alpha = 0.5
        return rotateRightButton
    }
    
    
    @objc func shieldsButton(rect: CGRect) -> UIButton {
        shieldsButton = UIButton(frame: CGRect(x: (rect.width / 2) - 35, y: rect.height - 90, width: 70, height: 70))
        shieldsButton.setTitle("SHIELDS", for: .normal)
        shieldsButton.titleLabel?.font = UIFont(name: "DINCondensed-Bold", size: 20)
        shieldsButton.titleLabel?.numberOfLines = 0
        shieldsButton.titleLabel?.textAlignment = .center
        shieldsButton.setTitleColor(.black, for: .normal)
        shieldsButton.titleEdgeInsets = UIEdgeInsets(top: 5, left: 0, bottom: 0, right: 0)
        shieldsButton.setBackgroundImage(UIImage(named: "JoyStickBase")!, for: .normal)
        shieldsButton.addTarget(self, action: #selector(self.shieldsPressed), for: .touchDown)
        shieldsButton.addTarget(self, action: #selector(self.shieldsReleased), for: .touchUpInside)
        shieldsButton.alpha = 0.5
        return shieldsButton
    }
    
    @objc func enterButton(rect: CGRect) -> UIButton {
        enterButton = UIButton(frame: CGRect(x: rect.width - 90, y: 10, width: 50, height: 30))
        enterButton.setTitle(" START ", for: .normal)
        enterButton.titleLabel?.font = UIFont(name: "DINCondensed-Bold", size: 20)
        enterButton.titleLabel?.textAlignment = .center
        enterButton.setTitleColor(.black, for: .normal)
        enterButton.titleEdgeInsets = UIEdgeInsets(top: 5, left: 0, bottom: 0, right: 4)
        enterButton.addTarget(self, action: #selector(self.enterPressed), for: .touchDown)
        enterButton.addTarget(self, action: #selector(self.enterReleased), for: .touchUpInside)
        enterButton.layer.borderColor = UIColor.white.cgColor
        enterButton.layer.borderWidth = CGFloat(1)
        enterButton.backgroundColor = UIColor.white
        enterButton.alpha = 0.5
        return enterButton
    }
    
    @objc func joyStick(rect: CGRect) -> JoyStickView {
        let size = CGSize(width: 100.0, height: 100.0)
        let joystick1Frame = CGRect(origin: CGPoint(x: 50.0,
                                                    y: (rect.height - size.height - 50.0)),
                                    size: size)
        joystickView = JoyStickView(frame: joystick1Frame)
        joystickView.delegate = self
        
        joystickView.movable = false
        joystickView.alpha = 0.5
        joystickView.baseAlpha = 0.5 // let the background bleed thru the base
        joystickView.handleTintColor = UIColor.darkGray // Colorize the handle
        return joystickView
    }
    
    @objc func firePressed(sender: UIButton!) {
        Key_Event(key: SDLK_SPACE, pressed: true)
    }
    
    @objc func fireReleased(sender: UIButton!) {
        Key_Event(key: SDLK_SPACE, pressed: false)
    }
    
    @objc func thrustPressed(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_UP, pressed: true)
    }
    
    @objc func thrustReleased(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_UP, pressed: false)
    }
    
    @objc func rotateLeftPressed(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_LEFT, pressed: true)
    }
    
    @objc func rotateLeftReleased(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_LEFT, pressed: false)
    }
    
    @objc func rotateRightPressed(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_RIGHT, pressed: true)
    }
    
    @objc func rotateRightReleased(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_RIGHT, pressed: false)
    }
    
    @objc func shieldsPressed(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_DOWN, pressed: true)
    }
    
    @objc func shieldsReleased(sender: UIButton!) {
        Scancode_Event(scancode: SDL_SCANCODE_DOWN, pressed: false)
    }
    
    @objc func enterPressed(sender: UIButton!) {
        Key_Event(key: SDLK_1, pressed: true)
    }
    
    @objc func enterReleased(sender: UIButton!) {
        Key_Event(key: SDLK_1, pressed: false)
    }
        
    func Key_Event(key: Int, pressed: Bool) {
        var event = SDL_Event()
        event.type = pressed ? SDL_KEYDOWN.rawValue : SDL_KEYUP.rawValue
        event.key.keysym.sym = SDL_Keycode(key)
        event.key.state = Uint8(pressed ? SDL_PRESSED : SDL_RELEASED)
        SDL_PushEvent(&event)
    }
    
    func Scancode_Event(scancode: SDL_Scancode, pressed: Bool) {
        var event = SDL_Event()
        event.type = pressed ? SDL_KEYDOWN.rawValue : SDL_KEYUP.rawValue
        event.key.keysym.scancode = scancode
        event.key.state = Uint8(pressed ? SDL_PRESSED : SDL_RELEASED)
        SDL_PushEvent(&event)
    }

    @objc func toggleControls(_ hide: Bool) {
        self.fireButton.isHidden = hide
        self.thrustButton.isHidden = hide
        self.rotateLeftButton.isHidden = hide
        self.rotateRightButton.isHidden = hide
        self.shieldsButton.isHidden = hide
        self.joystickView.isHidden = hide
        self.enterButton.isHidden = !hide
    }
    
    @objc func hideAllControls(_ hide: Bool) {
        self.fireButton.isHidden = true
        self.thrustButton.isHidden = true
        self.rotateLeftButton.isHidden = true
        self.rotateRightButton.isHidden = true
        self.shieldsButton.isHidden = true
        self.joystickView.isHidden = true
        self.enterButton.isHidden = !hide
    }
    
    open override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    open override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
    }
}

extension SDL_uikitviewcontroller: JoystickDelegate {
    
    func handleJoyStickPosition(x: CGFloat, y: CGFloat) {
        
        if y > 0.25 {
            Scancode_Event(scancode: SDL_SCANCODE_UP, pressed: true)
        } else if y < -0.25 {
//            Scancode_Event(scancode: SDL_SCANCODE_DOWN, pressed: true)
        } else {
            Scancode_Event(scancode: SDL_SCANCODE_UP, pressed: false)
//            Scancode_Event(scancode: SDL_SCANCODE_DOWN, pressed: false)
        }
        
        if x > 0.25 {
            Scancode_Event(scancode: SDL_SCANCODE_RIGHT, pressed: true)
        } else if x < -0.25 {
            Scancode_Event(scancode: SDL_SCANCODE_LEFT, pressed: true)
        } else {
            Scancode_Event(scancode: SDL_SCANCODE_RIGHT, pressed: false)
            Scancode_Event(scancode: SDL_SCANCODE_LEFT, pressed: false)
        }
        
    }
    
    func handleJoyStick(angle: CGFloat, displacement: CGFloat) {
//        print("angle: \(angle) displacement: \(displacement)")
    }
    
}
