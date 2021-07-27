// @dart=2.9

import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter/material.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';
import 'package:flutterfire_samples/screens/feedback.dart';
import 'package:flutterfire_samples/screens/sign_in_screen.dart';
import 'package:flutterfire_samples/screens/piano.dart';
import 'package:flutterfire_samples/screens/ClassicalPiano.dart';
import 'package:flutterfire_samples/utils/authentication.dart';
import 'package:flutterfire_samples/widgets/app_bar_title.dart';
import 'package:flutterfire_samples/screens/ranks.dart';
import 'package:flutterfire_samples/screens/savedSongs.dart';
import 'package:flutterfire_samples/screens/studentLog.dart';

class UserInfoScreen extends StatefulWidget {
  const UserInfoScreen({Key key,  User user})
      : _user = user,
        super(key: key);

  final User _user;

  @override
  _UserInfoScreenState createState() => _UserInfoScreenState();
}

class _UserInfoScreenState extends State<UserInfoScreen> {
   bool _isEmailVerified;
   User _user;

  bool _verificationEmailBeingSent = false;
  bool _isSigningOut = false;

  Route _routeToSignInScreen() {
    return PageRouteBuilder(
      pageBuilder: (context, animation, secondaryAnimation) => SignInScreen(),
      transitionsBuilder: (context, animation, secondaryAnimation, child) {
        var begin = Offset(-1.0, 0.0);
        var end = Offset.zero;
        var curve = Curves.ease;

        var tween =
            Tween(begin: begin, end: end).chain(CurveTween(curve: curve));

        return SlideTransition(
          position: animation.drive(tween),
          child: child,
        );
      },
    );
  }

  @override
  void initState() {
    _user = widget._user;
    _isEmailVerified = _user.emailVerified;

    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: CustomColors.firebaseNavy,
       appBar: AppBar(
        elevation: 0,
        backgroundColor: CustomColors.firebaseNavy,

        title: AppBarTitle(),

           actions: <Widget>[
             TextButton(
                 style: ButtonStyle(
                   foregroundColor: MaterialStateProperty.all<Color>(Colors.blue),
                   overlayColor: MaterialStateProperty.resolveWith<Color>(
                         (Set<MaterialState> states) {
                       if (states.contains(MaterialState.hovered))
                         return Colors.blue.withOpacity(0.04);
                       if (states.contains(MaterialState.focused) ||
                           states.contains(MaterialState.pressed))
                         return Colors.blue.withOpacity(0.12);
                       return null; // Defer to the widget's default.
                     },
                   ),
                 ),
                 onPressed: () { Navigator.push(
                   context,
                   MaterialPageRoute(builder: (context) => studentLog(user: _user,)),
                 );},
                 child: Text('Switch to Student Mode',style: TextStyle(
    color: Colors.redAccent,
    fontSize: 14,
    ),)
             )
           ],),
    drawer: Drawer(

    child: SafeArea(

    child: ListView(
        children: <Widget>[
    Container(height: 20.0),

    ListTile(title: Text("Check student's ranks",style: TextStyle(fontSize: 16,
      color: CustomColors.firebaseGrey,
        )),
      trailing: IconButton(
        icon: Icon(
          Icons.looks_one,
          color: CustomColors.firebaseGrey,
          size: 40.0,

        ),
        onPressed: () async {
          // do something
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => ranksScreen(user: _user,)),
          );
        },
      ),),
      Divider(),
      ListTile(title: Text("Check your saved tracks",style: TextStyle(fontSize: 16,
          letterSpacing: 0.5,
          color: CustomColors.firebaseYellow),),
      trailing: IconButton(
        icon: Icon(
          Icons.audiotrack_outlined,
          color: CustomColors.firebaseYellow,
          size: 40.0,
        ),
        onPressed: () async {
          // do something
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => savedScreen(user: _user,)),
          );
        },
      ),),
    Divider(),
          ListTile(title: Text("Check lessons status",style: TextStyle(fontSize: 16,
            color: CustomColors.firebaseGrey,
          )),
            trailing: IconButton(
              icon: Icon(
                Icons.airline_seat_recline_normal,
                color: CustomColors.firebaseGrey,
                size: 40.0,

              ),
              onPressed: () async {
                // do something
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => feedbackScreen(user: _user,)),
                );
              },
            ),)
    ]),
    )),

      body: SafeArea(

          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            crossAxisAlignment: CrossAxisAlignment.center,

            children: [


              Text(
                'Welcome',
                style: TextStyle(
                  color: CustomColors.firebaseGrey,
                  fontSize: 45,
                ),
              ),
              SizedBox(height: 8.0),
              Text(
                widget._user.displayName,
                style: TextStyle(
                  color: CustomColors.firebaseYellow,
                  fontSize: 30,
                ),
              ),
              SizedBox(height: 30.0),
           Padding(
          padding: const EdgeInsets.fromLTRB(0, 0, 0, 20),
            child :Text(
            'Press the piano to begin lesson',
            style: TextStyle(
              color: Colors.white,
              fontSize: 20,
              letterSpacing: 0.5,
            )
            )
          ),
              GestureDetector(
              onTap: () {
               Navigator.push(
                  context,
                 // MaterialPageRoute(builder: (context) => piano(user: _user,)),
                 MaterialPageRoute(builder: (context) => ClassicalPiano(user: _user,)),
                );
        },
                // handle your image tap here
          child: Padding(
            padding: const EdgeInsets.fromLTRB(0, 0, 0, 20),
          child:  Image.asset(
            'assets/piano2.png',
            fit: BoxFit.cover, // this is the solution for border
            width: 560.0,
            height: 220.0,
          ),
        ),
              )
              ,
              SizedBox(height: 50.0),
              _isEmailVerified
                  ? Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        ClipOval(
                          child: Material(
                            color: Colors.greenAccent.withOpacity(0.6),
                            child: Padding(
                              padding: const EdgeInsets.all(4.0),
                              child: Icon(
                                Icons.check,
                                size: 20,
                                color: Colors.white,
                              ),
                            ),
                          ),
                        ),
                        SizedBox(width: 8.0),
                        Text(
                          'Email is verified',
                          style: TextStyle(
                            color: Colors.greenAccent,
                            fontSize: 20,
                            letterSpacing: 0.5,
                          ),
                        ),
                      ],
                    )
                  : Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        ClipOval(
                          child: Material(
                            color: Colors.redAccent.withOpacity(0.8),
                            child: Padding(
                              padding: const EdgeInsets.all(4.0),
                              child: Icon(
                                Icons.close,
                                size: 20,
                                color: Colors.white,
                              ),
                            ),
                          ),
                        ),
                        SizedBox(width: 8.0),
                        Text(
                          'Email is not verified',
                          style: TextStyle(
                            color: Colors.redAccent,
                            fontSize: 20,
                            letterSpacing: 0.5,
                          ),
                        ),
                      ],
                    ),
              SizedBox(height: 8.0),
              Visibility(
                visible: !_isEmailVerified,
                child: Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    _verificationEmailBeingSent
                        ? CircularProgressIndicator(
                            valueColor: AlwaysStoppedAnimation<Color>(
                              CustomColors.firebaseGrey,
                            ),
                          )
                        : ElevatedButton(
                            style: ButtonStyle(
                              backgroundColor: MaterialStateProperty.all(
                                CustomColors.firebaseGrey,
                              ),
                              shape: MaterialStateProperty.all(
                                RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(10),
                                ),
                              ),
                            ),
                            onPressed: () async {
                              setState(() {
                                _verificationEmailBeingSent = true;
                              });
                              await _user.sendEmailVerification();
                              setState(() {
                                _verificationEmailBeingSent = false;
                              });
                            },
                            child: Padding(
                              padding: EdgeInsets.only(top: 8.0, bottom: 8.0),
                              child: Text(
                                'Verify',
                                style: TextStyle(
                                  fontSize: 20,
                                  fontWeight: FontWeight.bold,
                                  color: CustomColors.firebaseNavy,
                                  letterSpacing: 2,


                                ),
                              ),
                            ),
                          ),
                    SizedBox(width: 16.0),
                    IconButton(
                      icon: Icon(Icons.refresh),
                      onPressed: () async {
                        User user = await Authentication.refreshUser(_user);

                        if (user != null) {
                          setState(() {
                            _user = user;
                            _isEmailVerified = user.emailVerified;
                          });
                        }
                      },
                    ),
                  ],
                ),
              ),
              SizedBox(height: 24.0),
              Text(
                'You are now signed in using Firebase Authentication. To sign out of your account click the "Sign Out" button below.',
                style: TextStyle(
                    color: CustomColors.firebaseGrey.withOpacity(0.8),
                    fontSize: 14,
                    letterSpacing: 0.2),
              ),
              SizedBox(height: 16.0),
              _isSigningOut
                  ? CircularProgressIndicator(
                      valueColor: AlwaysStoppedAnimation<Color>(
                        Colors.redAccent,
                      ),
                    )
                  : ElevatedButton(
                      style: ButtonStyle(
                        backgroundColor: MaterialStateProperty.all(
                          Colors.redAccent,
                        ),
                        shape: MaterialStateProperty.all(
                          RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(10),
                          ),
                        ),
                      ),
                      onPressed: () async {
                        setState(() {
                          _isSigningOut = true;
                        });
                        await FirebaseAuth.instance.signOut();
                        setState(() {
                          _isSigningOut = false;
                        });
                        Navigator.of(context)
                            .pushReplacement(_routeToSignInScreen());
                      },
                      child: Padding(
                        padding: EdgeInsets.only(top: 8.0, bottom: 8.0),
                        child: Text(
                          'Sign Out',
                          style: TextStyle(
                            fontSize: 20,
                            fontWeight: FontWeight.bold,
                            color: Colors.white,
                            letterSpacing: 2,
                          ),
                        ),
                      ),
                    ),
            ],
          ),

      ),
    );
  }
}
