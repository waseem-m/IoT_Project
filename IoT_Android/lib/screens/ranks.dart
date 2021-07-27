// @dart=2.9

import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';


class ranksScreen extends StatefulWidget
{
  static const routeName = '/ranks';
  const ranksScreen({Key key,  User user})
      : _user = user,

        super(key: key);
  final User _user;

  @override
  _ranksState createState() => _ranksState();
}

class _ranksState extends State<ranksScreen> {
  int c=1;
  @override
  Widget build(BuildContext context) {
    CollectionReference users = FirebaseFirestore.instance.collection('Students');
    // collection('Users').document('uidHere').collection('destination').snapshots()
    return Scaffold(
      appBar: AppBar(
          backgroundColor: CustomColors.firebaseOrange,
          title: Text('Students Points'),
          actions: <Widget>[
            IconButton(
              icon: Icon(
                Icons.arrow_back_sharp,
                color:Colors.white ,
              ),
              onPressed: () async {
                // do something
                Navigator.pop(context);
              },
            ),
          ]
      ),
      body : SingleChildScrollView( child: new StreamBuilder<QuerySnapshot>(
        //stream: users.where("teacherUid", isEqualTo: widget._user.uid).snapshots(),
        stream: users.orderBy('points',descending: true).snapshots(),
        builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
          if (snapshot.hasError) {
            return Text('Something went wrong ');
          }

          if (snapshot.connectionState == ConnectionState.waiting) {
            return Center(
              child: CircularProgressIndicator(),
            );
          }

          final posts = snapshot.data.docs.asMap();

          c++;
          return Container(

            child: DataTable(
              columns: const <DataColumn>[
                DataColumn(label: Text('Rank')),
                DataColumn(label: Text('Student Email')),
                DataColumn(label: Text('Points')),

              ],
              //todo: fix the c counter
              rows: posts.entries
                  .map((e) => DataRow(cells: [
                DataCell(Text((e.key + 1).toString(),
                    style: TextStyle(
                    color:Colors.white,fontWeight: FontWeight.bold),),),
                DataCell(Text(e.value['studentEmail'])),
                DataCell(Text(e.value['points'].toString())),

              ] , color: MaterialStateProperty.resolveWith<Color>((Set<MaterialState> states) {

                if(e.key==0)
                  return Theme.of(context).colorScheme.surface.withRed(10).withGreen(90).withBlue(20);
                if(e.key==1)
                  return Theme.of(context).colorScheme.surface.withBlue(50).withGreen(110);
                if(e.key==2)
                  return Theme.of(context).colorScheme.surface.withGreen(180);
                //return null;  // Use the default value.
              }),))
                  .toList(),
            ),
          );


        },
      ),),

    );
  }
}