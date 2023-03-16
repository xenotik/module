CREATE TABLE USR_BUFFER.control_type_event
(
    id NUMBER PRIMARY KEY NOT NULL,
    name VARCHAR2(4000 CHAR) NOT NULL
);

insert into USR_BUFFER.public.control_type_event (id, name) values (1, 'Сообщение');
insert into USR_BUFFER.public.control_type_event (id, name) values (2, 'Выход абонента на сеанс');

CREATE TABLE USR_BUFFER.for_control
(
    id NUMBER PRIMARY KEY NOT NULL,
    code_of_event NUMBER NOT NULL,
    event_date timestamp NOT NULL,
    data_in_json VARCHAR2(4000 CHAR),
    CONSTRAINT event_types_fkey FOREIGN KEY (code_of_event)  REFERENCES USR_BUFFER.dict_type_event (ID)
);


CREATE SEQUENCE USR_BUFFER.SEQ_BUFFER_for_control

  MINVALUE 1
  MAXVALUE 99999999999999999999999999999999999
  START WITH 1
  INCREMENT BY 1
  CYCLE;

	
create or replace TRIGGER usr_BUFFER.TRG_BUFFER_new_tlg
after INSERT or UPDATE of TLG_ROUTE_STATUS, TLG_HEADERINPUTED, TLG_TO_ID, TLG_FROM_ID, TLG_ERRORCODE, TLG_URGENCY
on USR_BUFFER.TB_BUFFER_TLG
FOR EACH ROW

DECLARE
urgency   NVARCHAR2(250 CHAR);
name_abonent_in NVARCHAR2(250 CHAR); 
name_abonent_out NVARCHAR2(250 CHAR);

BEGIN
    select AB.ABONENT_NAME
	into name_abonent_in
	from usr_BUFFER.TB_BUFFER_ABONENTS AB
	where AB.ABONENT_ID = :NEW.TLG_FROM_ID;

    select AB.ABONENT_NAME
	into  name_abonent_out
	from usr_BUFFER.TB_BUFFER_ABONENTS AB
	where AB.ABONENT_ID = :NEW.TLG_TO_ID;

    IF(:NEW.TLG_URGENCY = 77) 
        then urgency := 'false';
        else urgency := 'true';
    end IF;

	case :NEW.TLG_DIRECTION
		   when 3 or 5 then
				insert into usr_BUFFER.for_control (ID, code_of_event, event_date, data_in_json) 
				values (SEQ_BUFFER_for_control.NEXTVAL, 1, sysdate, '{"IsUrgent":"'|| to_char(urgency) || '", '
																	||  '"id":"'|| CAST (:NEW.TLG_ID AS VARCHAR2(100))|| '", ' 
																	|| '"header":"'|| to_char(:NEW.TLG_HEADERINPUTED) || '", '
																	|| '"Sender":"'|| to_char(name_abonent_in) || '", '
																	|| '"SendDate":"'|| to_char(:NEW.TLG_INPUT_DATE) || '", '
                                                                    || '"ErrCode":"'|| to_char(:NEW.TLG_ERRORCODE) || '", '
																	|| '"Exchangestatus":"'|| to_char(:NEW.TLG_ROUTE_STATUS) || '"'
																	|| '}');
		   when 7 or 9 then
				insert into usr_BUFFER.for_control (ID, code_of_event, event_date, data_in_json) 
				values (SEQ_BUFFER_for_control.NEXTVAL, 1, sysdate,'{"IsUrgent":"'|| to_char(urgency) || '", '
																	||  '"id":"'|| CAST (:NEW.TLG_ID AS VARCHAR2(100))|| '", ' 
																	|| '"header":"'|| to_char(:NEW.TLG_HEADERINPUTED) || '", '
																	|| '"Receiver":"'|| to_char(name_abonent_out) || '", '
																	|| '"RecvDate":"'|| to_char(:NEW.TLG_OUTPUT_DATE) || '", '
                                                                    || '"ErrCode":"'|| to_char(:NEW.TLG_ERRORCODE) || '", '
																	|| '"Exchangestatus":"'|| to_char(:NEW.TLG_ROUTE_STATUS) || '"'
																	|| '}');
            else 
            	insert into usr_BUFFER.for_control (ID, code_of_event, event_date, data_in_json) 
				values (SEQ_BUFFER_for_control.NEXTVAL, 1, sysdate,'{"IsUrgent":"'|| to_char(urgency) || '", '
																	||  '"id":"'|| CAST (:NEW.TLG_ID AS VARCHAR2(100))|| '", ' 
																	|| '"header":"'|| to_char(:NEW.TLG_HEADERINPUTED) || '", '
                                                                    || '"ErrCode":"'|| to_char(:NEW.TLG_ERRORCODE) || '", '
																	|| '"Exchangestatus":"'|| to_char(:NEW.TLG_ROUTE_STATUS) || '"'
																	|| '}');
	  END CASE;
END TRG_BUFFER_new_tlg;
/
