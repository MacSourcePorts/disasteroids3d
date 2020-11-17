//
//  AboutView.swift
//  disasteroids3d-iOS
//
//  Created by Tom Kidd on 11/16/20.
//  Copyright © 2020 Tom Kidd. All rights reserved.
//

import UIKit

class AboutView: UIView {

    /*
    // Only override draw() if you perform custom drawing.
    // An empty implementation adversely affects performance during animation.
    override func draw(_ rect: CGRect) {
        // Drawing code
    }
    */

    @IBAction func dismissView(_ sender: Any) {
        self.isHidden = true
    }
    
    @IBAction func homePage(_ sender: Any) {
        if let url = URL(string: "https://www.lmnopc.com/disasteroids3d/") {
            UIApplication.shared.open(url)
        }
    }
    
    @IBAction func gitHub(_ sender: Any) {
        if let url = URL(string: "https://github.com/tomkidd/disasteroids3d/") {
            UIApplication.shared.open(url)
        }
    }
    
    @IBAction func emailTom(_ sender: Any) {
        if let url = URL(string: "mailto:tomkidd@gmail.com") {
            UIApplication.shared.open(url)
        }
    }
    
    @IBAction func emailThom(_ sender: Any) {
        if let url = URL(string: "mailto:thom.wetzel@gmail.com") {
            UIApplication.shared.open(url)
        }
    }
    

    
}
