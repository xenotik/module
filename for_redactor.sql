-- Table: public.for_control
DROP TABLE IF EXISTS public.dict_type_event;
DROP TABLE IF EXISTS public.control_type_event;
CREATE TABLE control_type_event
(
    id NUMBER PRIMARY KEY NOT NULL,
    name VARCHAR2(4000 CHAR) NOT NULL
);

insert into public.control_type_event (id, name) values (1, 'Сообщение');
insert into public.control_type_event (id, name) values (2, 'Выход абонента на сеанс');


DROP TABLE IF EXISTS public.for_control;
CREATE TABLE IF NOT EXISTS for_control
(
    id integer NOT NULL serial primary key,
    code_of_event integer NOT NULL,
    event_date timestamp without time zone NOT NULL,
    data_in_json text COLLATE pg_catalog."default",
    CONSTRAINT event_types_fkey FOREIGN KEY (code_of_event)
        REFERENCES public.control_type_event (id) MATCH SIMPLE
);



--TLG
DROP TRIGGER IF EXISTS new_tlg ON public.tlg;

DROP FUNCTION IF EXISTS public.new_tlg();

CREATE OR REPLACE FUNCTION public.new_tlg()
    RETURNS trigger
    LANGUAGE 'plpgsql'
    COST 100
    VOLATILE NOT LEAKPROOF
AS $BODY$

declare
abonent_in bool; abonent_out bool;
name_abonent_in varchar(4000); name_abonent_out  varchar(4000);
urgency  varchar(15);
BEGIN
	select iscurrentabonent, name
	into abonent_in, name_abonent_in
	from abonents
	where id = new.tlg_from;
	
	select iscurrentabonent, name
	into abonent_out, name_abonent_out
	from abonents
	where id = new.tlg_to;

	select case when NEW.urgency_id = 77 then 'false' else 'true' end
	into urgency
	from tlg;
			
	IF(abonent_in) then
		insert into for_control (code_of_event, event_date, data_in_json) 
		values ( 1, current_timestamp, concat('{"IsUrgent":"', urgency, '", ',
																  '"id":"', NEW.id , '", ',
																  '"header":"', NEW.header , '", ',
																  '"Sender":"', name_abonent_out , '", ',
																  '"Exchangestatus":"', NEW.exchangestatus , '"',
																  '}'));
	ELSEIF(abonent_out)	then
		insert into for_control (code_of_event, event_date, data_in_json) 
		values ( 1, current_timestamp, concat('{"IsUrgent":"', urgency, '", ',
																  '"id":"', NEW.id , '", ',
																  '"header":"', NEW.header , '", ',
																  '"receiver":"', name_abonent_in , '", ',
																  '"Exchangestatus":"', NEW.Exchangestatus , '"',
																  '}'));
	end if;
	RETURN NEW;
	END;
$BODY$;

CREATE TRIGGER new_tlg
    AFTER INSERT or UPDATE of header, tlg_from, tlg_to, exchangestatus, urgency_id
    ON public.tlg
    FOR EACH ROW
    EXECUTE FUNCTION public.new_tlg();
	
	
	
--public.abonent_state

DROP TRIGGER IF EXISTS new_session ON public.abonent_state;

DROP FUNCTION IF EXISTS public.new_session();

CREATE OR REPLACE FUNCTION public.new_session()
    RETURNS trigger
    LANGUAGE 'plpgsql'
    COST 100
    VOLATILE NOT LEAKPROOF
AS $BODY$
declare
BEGIN
	case when New.State = 3  and old.State != 3 then
		insert into for_control (code_of_event, event_date, data_in_json) 
		values ( 2, current_timestamp, concat('{"abonent_id":"', NEW.abonent_id , '", ',
																  '"session_start_date":"', NEW.session_start_date , '"',
																  '}'));
	when old.State = 3 and New.State != 3 then
		insert into for_control (code_of_event, event_date, data_in_json) 
		values ( 2, current_timestamp, concat('{"abonent_id":"', NEW.abonent_id , '", ',
																  '"session_end_date":"', NEW.session_end_date , '"',
																  '}'));
	
	end case;
	RETURN NEW;
END;
$BODY$;
	
CREATE TRIGGER new_session
    BEFORE UPDATE 
    ON public.abonent_state
    FOR EACH ROW
    EXECUTE FUNCTION public.new_session();
	
	
	CREATE OR REPLACE FUNCTION public.tlg_latest()
    RETURNS trigger
    LANGUAGE 'plpgsql'
    COST 100
    VOLATILE NOT LEAKPROOF
AS $BODY$

declare
abonent_in bool; abonent_out bool;
name_abonent_in varchar(4000); name_abonent_out  varchar(4000);
urgency_id  varchar(15);
BEGIN
	select iscurrentabonent, name
	into abonent_in, name_abonent_in
	from abonents
	where id = new.tlg_from;
	
	select iscurrentabonent, name
	into abonent_out, name_abonent_out
	from abonents
	where id = new.tlg_to;

	select case when NEW.urgency_id = 77 then 'false' else 'true' end
			into urgency_id
			from tlg;
	IF(abonent_in) then
		insert into for_control (code_of_event, event_date, data_in_json) 
		values ( 1, current_timestamp, concat('{"IsUrgent":"', urgency_id, '", ',
																  '"id":"', NEW.id , '", ',
																  '"header":"', NEW.header , '", ',
																  '"Sender":"', name_abonent_out , '", ',
																  '"SendDate":"', NEW.inputdate , '", ',
																  '"Exchangestatus":"', NEW.exchangestatus , '",',
																  '"Latest":"', NEW.latestatus , '"',
																  '}'));
	ELSEIF(abonent_out)	then
		insert into for_control (code_of_event, event_date, data_in_json) 
		values ( 1, current_timestamp, concat('{"IsUrgent":"', urgency_id, '", ',
																  '"id":"', NEW.id , '", ',
																  '"header":"', NEW.header , '", ',
																  '"receiver":"', name_abonent_in , '", ',
																  '"RecvDate":"', NEW.inputdate , '", ',
																  '"Exchangestatus":"', NEW.Exchangestatus , '",',
																  '"Latest":"', NEW.latestatus , '"',
																  '}'));
	end if;
	RETURN NEW;
	END;

	
$BODY$;

CREATE TRIGGER tlg_latest
    AFTER UPDATE OF latestatus
    ON public.tlg
    FOR EACH ROW
    EXECUTE FUNCTION public.tlg_latest();