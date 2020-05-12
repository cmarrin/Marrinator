//
//  ViewController.swift
//  Simulator
//
//  Created by Chris Marrin on 5/9/20.
//  Copyright © 2020 MarrinTech. All rights reserved.
//

import Cocoa

class ViewController: NSViewController {

    @IBOutlet weak var LED1: NSButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        LED1.state = NSControl.StateValue.on

        let bundle = Bundle.main
        let dir = bundle.path(forResource: "m8rFSFile", ofType:"")

        let fileManager = FileManager.default
        let destDir = NSHomeDirectory() + "/m8rFSFile"
        if (!fileManager.fileExists(atPath: destDir)) {
            do {
                try fileManager.copyItem(atPath: dir ?? "", toPath: destDir)
            }
            catch let error as NSError {
                print("Ooops! Something went wrong: \(error)")
            }
        }

        m8rInitialize(destDir)
        
        Timer.scheduledTimer(withTimeInterval: 0.05, repeats: true) { timer in
            m8rRunOneIteration()
        }
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }


}

